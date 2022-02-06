
#include "creature.hpp"
#include "config.hpp"
#include "world.hpp"

#include <algorithm>



void creature_t::interrupt(uint8_t n) noexcept
{
	if (this->ai.interrupt_index == 0)
	{
		this->ai.stack[0] = 0;
		this->ai.stack[1] = this->ai.ip;
	}

	this->ai.interrupt_index = n;
	this->ai.ip = 0;
}

void creature_t::interrupt_return() noexcept
{
	this->ai.interrupt_index = this->ai.stack[0];
	this->ai.ip = this->ai.stack[1];
}

creature_t::creature_t() noexcept
{
	memset(this, 0, sizeof(*this));
	//memset(this->ai.code[0], 1, sizeof(this->ai.code[0]));
	for (auto& b : this->ai.code[0])
		b = rand() & 0xFF;

	std::copy(config::cell_color, config::cell_color + 3, this->color);
	this->ai.code[1][0] = 5;
	this->postinit();
}

creature_t::creature_t(const creature_t& other) noexcept
{
	memcpy(this->ai.code, other.ai.code, sizeof(creature_t::ai.code));
	std::copy(other.color, other.color + 3, this->color);
	this->postinit();
}

void creature_t::postinit() noexcept
{
	this->ttl = rand() % (config::ttl_max - config::ttl_min + 1) + config::ttl_min;
	this->health = config::initial_health;
	this->energy = config::initial_energy;
	this->hibernate = 0;
	this->ai.interrupt_index = 0;
	this->ai.ip = 0;
	memset(this->ai.regs, 0, sizeof(this->ai.regs));
	memset(this->ai.stack, 0, sizeof(this->ai.stack));
}

static int mod(int a, int b) noexcept
{
	if (a < 0)
		a -= a / b * b - b;
	return a % b;
}

void creature_t::tick(world_t* world) noexcept
{
	if (this->hibernate)
	{
		--this->hibernate;
		return;
	}

	bool skip_ip_move = false;

	this->ai.ip &= 63;
	uint8_t* p = this->ai.code[this->ai.interrupt_index] + this->ai.ip;
	//if (*p & 0b10000000)
	//{
		//int8_t x = (int8_t)*p;
		//x <<= 1;
		//x >>= 1;
		//this->ai.ip += x - 1;
		//skip_ip_move = true;
	//}
	//else
	{
		switch (*p >> 3)
		{
		case 0:
			switch (*p & 7)
			{
			case 0:
				break; //nop

			case 1: //photosynthesis
				this->energy += config::photosynthesis_const * world->sun_level(this->pos) + 0.5f;
				break;

			case 2: //multiply
				//break;
				if (this->energy >= config::reproduce_energy)
				{
					world->place_creature_reproduce(*this);
					this->energy -= config::reproduce_energy;
				}
				break;

			case 3: //IP = X
				this->ai.ip = *++p;
				skip_ip_move = true;
				break;

			case 4: //IP += X
				this->ai.ip += *++p;
				skip_ip_move = true;
				break;

			case 5: //return from interrupt
				skip_ip_move = true;
				this->interrupt_return();
				break;

			case 6: //heal
				++this->health;
				this->energy -= config::heal_cost;
				break;

			case 7: //eat from here
				this->energy += world->eat(this->pos);
				break;
			}
			break;
		
		case 1: //walk
			world->move_creature(this->pos, this->pos + dpos[*p & 7]);
			break;

		case 2: //store value
		{
			uint8_t x = p[1];
			this->ai.regs[*p & 7] = x;
			++p;
		}
		break;

		case 3: //increment value
			this->ai.regs[*p & 7]++;
			break;

		case 4: //decrement value
			this->ai.regs[*p & 7]--;
			break;

		case 5: //relative jump if < 0
			if (this->ai.regs[*p & 7] < 0)
			{
				this->ai.ip += p[1];
				skip_ip_move = true;
			}
			++p;
			break;

		case 6: //relative jump if == 0
			if (this->ai.regs[*p & 7] == 0)
			{
				this->ai.ip += p[1];
				skip_ip_move = true;
			}
			++p;
			break;

		case 7: //relative jump if == const
			if (this->ai.regs[*p & 7] == p[2])
			{
				this->ai.ip += p[1];
				skip_ip_move = true;
			}
			p += 2;
			break;

		case 8: //eat from side
			this->energy += world->eat(this->pos + dpos[*p & 7]);
			break;

		case 9: //attack
			world->attack(this->pos + dpos[*p & 7]);
			break;

		case 10: //look
		{
			const uint8_t dst = *p & 7;
			const uint8_t byte2 = *++p;
			const uint8_t dir = byte2 >> 5;
			const uint8_t length = byte2 & 32;
			const pos_t delta = dpos[dir];
			world_entry* entry = nullptr;
			bool have_lock = false;

			uint8_t result = object_type_none;
			pos_t pos = this->pos;
			for (uint8_t i = 0; i < length; ++i)
			{
				pos += delta;

				entry = world->at(pos);
				if (!entry)
					break;

				entry->sema.acquire_for_read();
				have_lock = true;
				auto type = entry->type;

				if (type != object_type_none)
				{
					result = type;
					break;
				}
				else if (entry->energy != 0)
				{
					result = object_type_food;
					break;
				}
				entry->sema.release_from_read();
				have_lock = false;
			}

			if (result == object_type_stranger)
			{
				const uint8_t* my_code = (uint8_t*)this->ai.code;
				const uint8_t* other_code = (uint8_t*)entry->p_creature->ai.code;

				int diff = 0;
				(void)std::any_of((uint8_t*)this->ai.code, (uint8_t*)std::end(this->ai.code), [&]
				(const uint8_t& byte)
				{
					if (byte != other_code[&byte - my_code])
					{
						if (++diff >= config::stranger_threshold)
							return false;
					}
					return true;
				});


				if (diff < config::stranger_threshold)
					result = object_type_relative;
			}

			if (have_lock)
				entry->sema.release_from_read();

			this->ai.regs[dst] = result;
		}
		break;

		case 11: //count objects of type T

			break;

		case 12: //find nearest object of type T

			break;

		case 13: //walk to storage[x]
			world->move_creature(this->pos, this->pos + dpos[this->ai.regs[*p & 7] & 7]);
			break;

		case 14: //eat from storage[x]
			this->energy += world->eat(this->pos + dpos[this->ai.regs[*p & 7] & 7]);
			break;

		case 15: //attack storage[x]
			world->attack(this->pos + dpos[this->ai.regs[*p & 7] & 7]);
			break;

		case 16: //random walk
		{
			uint8_t arr[8];
			uint8_t* p = arr;
			const uint8_t dirs = *++p;

			for (size_t i = 0; i < 8; ++i)
				if (!(dirs & (1 << i)))
					*p++ = (uint8_t)i;

			if (p == arr)
				break;
			world->move_creature(this->pos, this->pos + dpos[arr[rand() % (p - arr)]]);
		}
		break;

		case 17: //storage[X] = energy level / 10
		{
			int energy_val = (int)this->energy;
			energy_val /= 10;
			++energy_val;
			if (energy_val > 255)
				energy_val = 255;
			this->ai.regs[*p & 7] = (uint8_t)energy_val;
		}
		break;

		case 18: //storage[X] = health level
			this->ai.regs[*p & 7] = this->health;
			break;

		case 19: //storage[X] = OppositeDirection(storage[X])
			this->ai.regs[*p & 7] = abs(4 - this->ai.regs[*p & 7]);
			break;

		case 20: //storage[X] = LeftDirection(storage[X])
			this->ai.regs[*p & 7] -= 2;
			this->ai.regs[*p & 7] &= 7;
			break;

		case 21: //storage[X] = RightDirection(storage[X])
			this->ai.regs[*p & 7] += 2;
			this->ai.regs[*p & 7] &= 7;
			break;

		case 22: //storage[X] = HalfLeftDirection(storage[X])
			this->ai.regs[*p & 7] -= 1;
			this->ai.regs[*p & 7] &= 7;
			break;

		case 23: //storage[X] = HalfRightDirection(storage[X])
			this->ai.regs[*p & 7] += 1;
			this->ai.regs[*p & 7] &= 7;
			break;

		case 24: //storage[X] = random integer from [0; A]
		{
			int A = p[1];
			this->ai.regs[*p & 7] = rand() % (A + 1);
			++p;
		}
		break;

		case 25: //storage[X] = sun level 
			this->ai.regs[*p & 7] = (uint8_t)world->sun_level(this->pos);
			break;
		}
	}

	if (!skip_ip_move)
		this->ai.ip = (uint8_t)(p - this->ai.code[this->ai.interrupt_index] + 1);

	this->energy -= config::energy_base_cost;
	--this->ttl;

	//auto kill_self = [&]
	//(float energy)
	//{
	//	auto* entry = world->at(this->pos);
	//	entry->p_creature = nullptr;
	//	entry->type = object_type_none;
	//	entry->energy += energy + config::lefover_body_energy;
	//	delete this;
	//};

	if (this->energy <= 0)
	{
		world->kill_creature(this->pos);
		//kill_self(this->energy);
	}
	else if (this->ttl == 0)
	{
		if (this->energy >= config::reproduce_energy)
		{
			world->place_creature_reproduce(*this);
			world->place_creature_reproduce(*this);
			this->energy -= config::reproduce_energy;
		}
		//kill_self(this->energy);
		world->kill_creature(this->pos);
	}
}

void creature_t::interrupt_attacked() noexcept
{
	this->interrupt(1);
}

void creature_t::mutate() noexcept
{
	for (int i = 0; i < 3; ++i)
	{
		size_t a = rand() % std::size(this->ai.code);
		size_t b = rand() % std::size(this->ai.code[0]);
		this->ai.code[a][b] = rand() & 0xFF;
	}
	
	for (auto& c : this->color)
		c = rand() & 0xFF;
}
