
#ifndef _WORLD_HPP_
#define _WORLD_HPP_


#include <stdint.h>

#include <forward_list>
#include <memory>
#include <memory>

#include <ksn/math_vec.hpp>

#include "creature.hpp"
#include "race_semaphore.hpp"



enum object_type : uint16_t
{
	object_type_none,
	object_type_food,
	object_type_relative,
	object_type_stranger,
	object_type_wall,
};



struct world_entry
{
	std::unique_ptr<creature_t> p_creature;
	//creature_t* p_creature;
	float energy;
	race_semaphore<uint16_t> sema;
	object_type type;
};

class world_t
{
public:
	using pos_t = ksn::vec<2, int32_t>;

	static constexpr size_t width = 80;
	static constexpr size_t height = 60;

	static constexpr bool looped = false;


private:

	void destroy_at(pos_t) noexcept;

	static bool valid_pos(pos_t&) noexcept;
	void kill_creature(world_entry&) noexcept;


public:

	world_entry world[width][height];

	void place_creature_create(pos_t pos) noexcept;
	void place_creature_init(pos_t pos, const creature_t&) noexcept;
	void place_creature_reproduce(const creature_t&) noexcept;
	void move_creature(pos_t from, pos_t to) noexcept;
	void attack(pos_t dst) noexcept;
	float eat(pos_t dst) noexcept;
	void kill_creature(pos_t dst) noexcept;

	world_entry* at(pos_t pos) noexcept;
	const world_entry* at(pos_t pos) const noexcept;

	float sun_level(pos_t at) const noexcept;

	pos_t pos(const world_entry&) const noexcept;

	world_t() noexcept;
	~world_t() noexcept;
};


#endif //!_WORLD_HPP_
