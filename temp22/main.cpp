
#include <stdint.h>
#include <assert.h>

#include <bit>
#include <random>
#include <print>

using u64 = uint64_t;
using u8 = uint8_t;

u64 bit_stream_prepend(u64 stream, u64 new_vals, u8 vals_size)
{
	return (stream << vals_size) | new_vals;
}
u64 bit_stream_append(u64 stream, u64 new_vals, u8 stream_size)
{
	return stream | (new_vals << stream_size);
}

void reduce_common_powers_of_two(u64& val, u8& max_shift)
{
	const u8 shifter = std::min(max_shift, (u8)std::countr_zero(val));
	val >>= shifter;
	max_shift -= shifter;
}

template<class Rng>
constexpr u8 get_rng_entropy(const Rng& rng)
{
	if constexpr (std::is_same_v<Rng, std::random_device>)
		return (u8)rng.entropy();
	else
	{
		const auto range = u64(Rng::max() - Rng::min());
		return std::bit_width(range);
	}
}

struct u128
{
	u64 x[2];

	static u128 from_u64_multiplication(u64 a, u64 b) noexcept
	{
		u128 result;
		result.x[0] = _umul128(a, b, &result.x[1]);
		return result;
	}

	u8 ceil_log2() const noexcept
	{
		if (x[1] != 0) [[likely]]
			return 64 + std::bit_width(x[1] - bool(x[0] == 0));

		if (x[0] != 0) [[likely]]
			return std::bit_width(x[0] - 1);

		return -1;
	}

	u8 get_u64_overflow_length() const noexcept
	{
		return std::bit_width(x[1]);
	}

	const u64& operator[](size_t n) const noexcept
	{
		return x[n];
	}

	u128& operator>>=(u8 bits) noexcept
	{
		if (bits >= 64) [[unlikely]]
		{
			x[0] = x[1] >> (bits - 64);
			x[1] = 0;
		}
		else
		{
			x[0] = (x[0] >> bits) | (x[1] << (64 - bits));
			x[1] >>= bits;
		}
		return *this;
	}

	u128& operator+=(u64 a) noexcept
	{
		x[1] += bool(x[0] + a < x[0]);
		x[0] += a;
		return *this;
	}
};

struct predetermined_rng
{
	const char* p;

	u64 operator()()
	{
		assert(*p == '0' || *p == '1');
		return *p++ == '1';
	}

	static u64 min() { return 0; }
	static u64 max() { return 1; }
};

template<class Rng>
struct ac_sampler
{
	ac_sampler(Rng& rng)
		: rng(rng), rng_entropy(get_rng_entropy(rng))
	{
	}

private:
	u64 bit_stream = 0;
	u64 state = 0; //actual value = stored / 2^64
	u64 t_num = 1;
	Rng& rng;
	u8 t_denom = 0; //t = t_num / 2^t_denom
	u8 bit_stream_size = 0;
	const u8 rng_entropy = 0;

	void retrieve_bits() noexcept
	{
		bit_stream = bit_stream_append(bit_stream, rng(), bit_stream_size);
		bit_stream_size += rng_entropy;
	}

	u64 get_n_bits(u8 n) noexcept
	{
		assert(n <= 64);

		if (n == rng_entropy) [[unlikely]]
			return (u64)rng();

		u64 result = 0;
		while (n > bit_stream_size) [[unlikely]]
		{
			retrieve_bits();
			if (bit_stream_size >= n) [[likely]]
				break;
			const u8 bits_available = bit_stream_size;
			result = merge_with_n_existing_bits(result, bits_available);
			n -= bits_available;
		}
		return merge_with_n_existing_bits(result, n);
	}

	u64 merge_with_n_existing_bits(u64 old_bits, u8 existing_size) noexcept
	{
		return bit_stream_prepend(old_bits, extract_existing_n_bits(existing_size), existing_size);
	}
	//u64 merge_with_n_existing_bits(u64 old_bits, u8 old_size, u8 existing_size) noexcept
	//{
	//	return bit_stream_append(old_bits, extract_existing_n_bits(existing_size), old_size);
	//}

	u64 extract_existing_n_bits(u8 n) noexcept
	{
		assert(n <= bit_stream_size);

		const u64 result = bit_stream & ((1ull << n) - 1);
		bit_stream >>= n;
		bit_stream_size -= n;
		return result;
	}

	static u8 clamped_subtraction(u8 a, u8 b)
	{
		return (a - b) * bool(a >= b);
	}
	static u64 safe_add(u64 a, u64 b)
	{
		assert(UINT64_MAX - a >= b);
		return a + b;
	}
	static u64 safe_mul(u64 a, u64 b)
	{
		assert(a == 0 || UINT64_MAX / a >= b);
		return a * b;
	}
	static u64 safe_lshift(u64 x, u8 s)
	{
		assert(std::countl_zero(x) >= s);
		return x << s;
	}
public:
	//TODO: calling with 0 assumes 2^64
	u64 operator()(u64 max) noexcept
	{
		double e1, e2, e3;
		e1 = get_stored_entropy();

		do
		{
			const u8 unshifted_b = u128::from_u64_multiplication(max, t_num).ceil_log2();
			if (t_denom >= unshifted_b)
				break;
			const u8 b = unshifted_b - t_denom;
			const u64 bits = get_n_bits(b);
			t_denom += b;

			u64 _ = safe_mul(bits, t_num);
			_ = safe_lshift(_, 64 - t_denom);
			state = safe_add(state, _);
			//state += bits * t_num << (64 - t_denom); //if the formula was correct, it would be impossible for overflow to occur
		} while (false);
		
		e2 = get_stored_entropy();

		const auto x = u128::from_u64_multiplication(state, max);
		state = x[0];

		reduce_common_powers_of_two(max, t_denom);

		{
			auto t = u128::from_u64_multiplication(t_num, max);
			const u8 overflow_shift = t.get_u64_overflow_length();
			if (overflow_shift > 0) //TODO: unlikely?
			{
				const bool should_round_up = t[0] & ((1ull << overflow_shift) - 1);
				t >>= overflow_shift;
				t += should_round_up;
				t_denom -= overflow_shift;
			}
			t_num = t[0];
		}

		e3 = get_stored_entropy();

		std::println("{:3}: {:.3f} {:.3f} {:.3f}", x[1], e1, e2, e3);
		
		return x[1];
	}

	double get_stored_entropy() const noexcept
	{
		return t_denom - log2l((double)t_num);
	}
};

int main()
{
	predetermined_rng prng{"0111110011000110100110011011011000110101110110101001101111110100"};
	ac_sampler urng(prng);

	u64 _ = 0;
	_ = urng(10);
	_ = urng(10);
	_ = urng(10);
	_ = urng(10);
	_ = urng(10); //breaks here
	_ = urng(10);
	_ = urng(10);
	_ = urng(10);
}
