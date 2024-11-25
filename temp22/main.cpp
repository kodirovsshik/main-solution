
#include <stdint.h>
#include <assert.h>

#include <vector>
#include <bit>
#include <random>
#include <numeric>
#include <ranges>

using in_t = uint8_t;
using out_t = uint8_t;

static constexpr size_t n = 4;
static constexpr size_t m = 5;

static constexpr size_t table_len = 1 << n;
static constexpr size_t aux_table_len = 1 << m;

using table_t = out_t[table_len];
using aux_table_t = out_t[aux_table_len];

std::mt19937_64 global_rng;

#define bit_distance(a, b) (size_t)std::popcount(uintmax_t((a) ^ (b)))

in_t decode(out_t val, const table_t& table)
{
	size_t min_distance = SIZE_MAX;
	in_t min_idx = 0;
	for (in_t i = 0; i < (in_t)table_len; ++i)
	{
		const size_t current_distance = bit_distance(val, table[i]);
		if (current_distance < min_distance)
		{
			min_distance = current_distance;
			min_idx = i;
		}
	}
	return min_idx;
}

size_t C(size_t n, size_t k)
{
	size_t result = 1;
	for (size_t i = n; i != n - k; --i)
		result = result * i / (n - i + 1);
	return result;
}

out_t* generate_bitmasks(out_t* p, size_t len, size_t set, out_t val = 0)
{
	if (set == 0)
	{
		*p = val;
		return p + 1;
	}
	if (set == 1)
	{
		for (size_t i = 0; i < len; ++i)
			*p++ = val | (out_t(1) << i);
		return p;
	}
	for (size_t top_bit_pos = set - 1; top_bit_pos < len; ++top_bit_pos)
	{
		p = generate_bitmasks(p, top_bit_pos, set - 1, val | (out_t(1) << top_bit_pos));
	}
	return p;
}
const auto& enumerate_n_bit_masks(size_t set_bits)
{
	static std::vector<out_t> masks[m + 1];

	assert(set_bits <= m);

	auto& v = masks[set_bits];
	if (v.size() != 0)
		return v;

	v.resize(C(m, set_bits));
	generate_bitmasks(v.data(), m, set_bits);
	return v;
}

size_t evaluate_table(const table_t& table)
{
	//evaluation = sum of misdecodings after (upto) *delta* bits are changed
	static constexpr size_t delta = 1;

	size_t result = 0;

	for (in_t i = 0; i != (in_t)table_len; ++i)
	{
		for (size_t j = 1; j <= delta; ++j)
		{
			for (auto mask : enumerate_n_bit_masks(j))
			{
				result += decode(table[i] ^ mask, table) != i;
			}
		}
	}

	return result;
}

template<class Rng>
class unbiased_rng_sampler
{
	Rng& rng;
	size_t bits;

public:
	unbiased_rng_sampler(Rng& rng)
		: rng(rng)
	{
		const auto val_range = uint64_t(Rng::max() - Rng::min());
	}
};

template<class T, class Rng>
void generate_range_sample(T* out, T min, T max, Rng&& rng = global_rng)
{
	unbiased_rng_sampler urng(rng);
}

void generate_table(table_t& table)
{
}

int main()
{
	table_t table;
	generate_table(table);
	
}
