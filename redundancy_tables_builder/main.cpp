
#include <stdint.h>

using in_t = uint8_t;
using out_t = uint8_t;

static constexpr size_t n = 4;
static constexpr size_t m = 5;

static constexpr size_t table_len = 1 << n;
using table_t = out_t[table_len];

#define bit_distance(a, b) (size_t)std::popcount((a) ^ (b))

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

template<class T = size_t>
class ascending_multiindex
{
	std::vector<T> idx;
	T max;

public:

	const T& operator[](size_t n) const noexcept
	{
		return idx[n];
	}
	size_t 
	
	ascending_multiindex(size_t size, T max) noexcept
		: max(max)
	{
		idx.resize(size);
	}
	
	void reset() noexcept
	{
		std::ranges::iota(idx, T{});
	}
	
	bool end_reached() const noexcept
	{
		return idx.empty() || idx.back() > max;
	}
	bool operator++() noexcept
	{
		size_t i;
		for (i = N - 1; i != -1; --i)
		{
			if (++idx[i] <= i + max - idx.size() + 1)
				break;
		}
		if (i == -1)
		{
			thus->reset();
			return false;
		}
		for (i = i + 1; i != N; ++i)
		{
			idx[i] = idx[i - 1] + 1;
		}
		return true;
	}
}

std::generator<out_t> enumerate_corruptions(out_t val, size_t len)
{
	ascending_multiindex idx(len, m - 1);
	
	while (!idx.end_reached())
	{
		
	}
}

size_t eval(const table_t& table)
{
	//evaluation = sum of misdecodings after (upto) *delta* bits are changed
	static constexpr size_t delta = 1;

	size_t result = 0;

	for (in_t i = 0; i != (in_t)table_len; ++i)
	{
		for (size_t j = 1; j <= delta; ++j)
		{
			for (auto corrupted : enumerate_corruptions(table[i], j))
			{
				result += decode(corrupted, table) != i;
			}
		}
	}
}

int main()
{

}
