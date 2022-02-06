
#ifndef _CREATURE_HPP_
#define _CREATURE_HPP_


#include <ksn/math_vec.hpp>



class world_t;

class creature_t
{
	using pos_t = ksn::vec<2, int32_t>;

	friend class world_t;

	struct
	{
		uint8_t code[2][256];
		uint8_t regs[8];
		uint8_t stack[2]; //for interrupt
		uint8_t ip;
		uint8_t interrupt_index;
	} ai;

	float energy;

	uint8_t health;
	uint8_t ttl;
	uint8_t hibernate;

public:
	uint8_t color[3];

private:

	void interrupt(uint8_t) noexcept;
	void interrupt_return() noexcept;

	void postinit() noexcept;

public:

	pos_t pos;


	creature_t() noexcept;
	creature_t(const creature_t&) noexcept;

	void tick(world_t* ptr) noexcept;
	void interrupt_attacked() noexcept;

	void mutate() noexcept;
};


#endif //!_CREATURE_HPP_
