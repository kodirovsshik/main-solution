
#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#pragma warning(disable : 26812)

#include <stdint.h>
#include <ksn/math_vec.hpp>


struct config
{
	static constexpr float lefover_body_energy = 10;

	static constexpr uint8_t initial_health = 1;
	static constexpr float initial_energy = 50;

	static constexpr float reproduce_energy = initial_energy + lefover_body_energy;

	static constexpr float cell_size = 10;

	static constexpr uint8_t cell_color[3] = { 255, 0, 0 };
	static constexpr uint8_t food_color[3] = { 0, 127, 0 };

	static constexpr float photosynthesis_const = 1;

	static constexpr float energy_base_cost = 1;

	static constexpr float heal_cost = lefover_body_energy;

	static float update_period;

	static constexpr uint8_t ttl_min = 240;
	static constexpr uint8_t ttl_max = 255;

	static constexpr float mutation_rate = 1.f / 10;

	static constexpr uint8_t stranger_threshold = 3;

	static constexpr float rebirth_rate = 1.f / 3;
};


static constexpr ksn::vec2i dpos[8] =
{
	{ -1, 1 },
	{ 0, 1, },
	{ 1, 1, },
	{ 1, 0, },
	{ 1, -1, },
	{ 0, -1, },
	{ -1, -1, },
	{ -1, 0, },
};


#endif //!_CONFIG_HPP_
