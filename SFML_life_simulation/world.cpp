
#include "world.hpp"

#include "config.hpp"

#include <algorithm>
#include <execution>
#include <random>

#include <Windows.h>



static int mod(int a, int b) noexcept
{
	if (a < 0)
		a += a / b * b;
	return a % b;
}

static float rand_float() noexcept
{
	return rand() / (float)RAND_MAX;
}



struct adopt_lock_t {} static adopt_lock;

template<bool write, bool try_only, std::integral T>
struct race_semaphore_sentry
{
private:
	race_semaphore<T>* p;

public:
	race_semaphore_sentry(race_semaphore<T>* p) noexcept
		: p(p)
	{
		bool ok;
		if (!p)
			return;

		if constexpr (try_only)
		{
			if constexpr (write)
				ok = p->try_acquire_for_write();
			else
				ok = p->try_acquire_for_read();
		}
		else
		{
			ok = true;
			if constexpr (write)
				p->acquire_for_write();
			else
				p->acquire_for_read();
		}

		
		if (!ok)
			this->p = nullptr;
	}
	race_semaphore_sentry(race_semaphore<T>* p, adopt_lock_t)
		: p(p)
	{
	}
	~race_semaphore_sentry() noexcept
	{
		this->release();
	}

	void release() noexcept
	{
		if (!p)
			return;

		if constexpr (write)
			p->release_from_write();
		else
			p->release_from_read();

		this->p = nullptr;
	}

	void forget() noexcept
	{
		this->p = nullptr;
	}

	bool has_lock() const noexcept
	{
		return this->p != nullptr;
	}

	race_semaphore_sentry(const race_semaphore_sentry&) = delete;
	race_semaphore_sentry(race_semaphore_sentry&&) = delete;

	race_semaphore_sentry& operator=(const race_semaphore_sentry&) = delete;
};

using _read_sentry = race_semaphore_sentry<false, false, uint16_t>;
using _write_sentry = race_semaphore_sentry<true, false, uint16_t>;
using _try_read_sentry = race_semaphore_sentry<false, true, uint16_t>;
using _try_write_sentry = race_semaphore_sentry<true, true, uint16_t>;


bool world_t::valid_pos(pos_t& p) noexcept
{
	if constexpr (world_t::looped)
	{
		p[0] = mod(p[0], world_t::width);
		p[1] = mod(p[1], world_t::height);
	}
	else
	{
		if (p[0] < 0 || p[1] < 0 || p[0] >= world_t::width || p[1] >= world_t::height)
			return false;
	}
	return true;
}

void world_t::destroy_at(pos_t p) noexcept
{
	if (!valid_pos(p))
		return;

	auto& entry = this->world[p[0]][p[1]];
	_write_sentry s(&entry.sema);

	//if (entry.p_creature)
		//delete entry.p_creature;

	entry.p_creature = nullptr;
	entry.type = object_type_none;
}


void world_t::place_creature_create(pos_t p) noexcept
{
	if (!valid_pos(p))
		return;

	auto& entry = this->world[p[0]][p[1]];
	_write_sentry s(&entry.sema);

	if (entry.type)
		return;

	entry.type = object_type_stranger;
	entry.p_creature = std::make_unique<creature_t>();
	entry.p_creature->pos = p;
}

void world_t::place_creature_init(pos_t p, const creature_t& other) noexcept
{
	if (!valid_pos(p))
		return;

	auto& entry = this->world[p[0]][p[1]];
	_write_sentry s(&entry.sema);

	if (entry.type)
		return;

	entry.type = object_type_stranger;
	entry.p_creature = std::make_unique<creature_t>(other);
	entry.p_creature->pos = p;
}

void world_t::place_creature_reproduce(const creature_t& obj) noexcept
{
	world_entry* entry = nullptr;
	auto pos = obj.pos;

	static std::minstd_rand random_engine;

	pos_t positions[8];
	std::transform(positions, positions + 8, dpos, positions, [&]
		(ksn::vec2i, ksn::vec2d dx) { return obj.pos + dx; });
	std::shuffle(positions, positions + 8, random_engine);

	for (const auto& new_pos : positions)
	{
		world_entry* current_entry = this->at(new_pos);
		if (!current_entry)
			continue;

		_try_write_sentry _wsentry(&current_entry->sema);
		if (!_wsentry.has_lock())
			continue;
		if (current_entry->type != object_type_none)
			continue;

		entry = current_entry;
		pos = new_pos;
		_wsentry.forget();
		break;
	}

	if (!entry)
		return;

	_write_sentry _wsentry(&entry->sema, adopt_lock);
	entry->type = object_type_stranger;
	entry->p_creature = std::make_unique<creature_t>(obj);
	entry->p_creature->pos = pos;

	if (rand_float() <= config::mutation_rate)
		entry->p_creature->mutate();
	entry->energy += 5;
}

void world_t::move_creature(pos_t p1, pos_t p2) noexcept
{
	if (!valid_pos(p1))
		return;
	if (!valid_pos(p2)) 
		return;

	auto& entry1 = this->world[p1[0]][p1[1]];
	auto& entry2 = this->world[p2[0]][p2[1]];

	_try_write_sentry _wsentry(&entry2.sema);
	if (!_wsentry.has_lock())
		return;

	if (entry2.type != object_type_none || entry1.type != object_type_stranger)
		return;

	std::swap(entry1.type, entry2.type);
	std::swap(entry1.p_creature, entry2.p_creature);

	entry2.p_creature->pos = p2;
}

void world_t::kill_creature(world_entry& entry) noexcept
{
	float energy = entry.p_creature->energy;
	if (energy < 0)
		energy = 0;

	_try_write_sentry _s(&entry.sema);
	if (_s.has_lock())
		__debugbreak();

	//auto pos = this->pos(entry);

	//auto p = entry.p_creature;
	entry.p_creature = nullptr;
	entry.type = object_type_none;
	//delete p;
	
	entry.energy += energy + config::lefover_body_energy;
}

void world_t::attack(pos_t p) noexcept
{
	if (!valid_pos(p))
		return;

	auto& entry = this->world[p[0]][p[1]];

	_try_write_sentry _wsentry(&entry.sema);
	if (!_wsentry.has_lock())
		return;

	if (entry.type != object_type_stranger)
		return;

	if (--entry.p_creature->health == 0)
		this->kill_creature(entry);
}

float world_t::eat(pos_t p) noexcept
{
	if (!valid_pos(p))
		return 0;

	auto& entry = this->world[p[0]][p[1]];

	_try_write_sentry _s(&entry.sema);
	if (!_s.has_lock())
		return 0;

	float result = entry.energy;
	entry.energy = 0;
	return result;
}

void world_t::kill_creature(pos_t p) noexcept
{
	if (!valid_pos(p))
		return;
	auto& entry = this->world[p[0]][p[1]];

	//_write_sentry _wsentry(&entry.sema);
	_try_write_sentry _wsentry(&entry.sema);
	//if (!_wsentry.has_lock())
		//return;
	this->kill_creature(entry);
}

const world_entry* world_t::at(pos_t p) const noexcept
{
	return ((world_t*)this)->at(p);
}
world_entry* world_t::at(pos_t p) noexcept
{
	if (!valid_pos(p))
		return nullptr;
	return &this->world[p[0]][p[1]];
}

float world_t::sun_level(pos_t at) const noexcept
{
	return 0;
	//float x = 2 * (at[0] / (float)this->width) - 1;
	//return 1.3f * expf(-3 * x * x);
}

world_t::pos_t world_t::pos(const world_entry& obj) const noexcept
{
	size_t delta = &obj - (world_entry*)this->world;
	return pos_t{ delta / height, delta % height };
}

world_t::~world_t() noexcept
{
	using column_t = world_entry[height];

	//std::for_each(std::execution::par_unseq, this->world, std::end(this->world), []
	//(column_t& column)
	//{
	//	for (auto& entry : column)
	//	{
	//		if (entry.type == object_type_stranger)
	//			delete entry.p_creature;
	//		entry.type = object_type_none;
	//	}
	//});
}

world_t::world_t() noexcept
{
	memset(this->world, 0, sizeof(this->world));
}
