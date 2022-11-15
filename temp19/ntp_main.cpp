
#include <iostream>
#include <chrono>
#include <fstream>
#include <array>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>
#include <functional>
#include <variant>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <span>
#include <random>

#include <ctype.h>
#include <stdint.h>

template<class T>
constexpr unsigned log2_of_rounded_up_to_pow2(T x)
{
	unsigned result = 1;
	while (x > 1)
	{
		x /= 2;
		++result;
	}
	return result;
}

template<class T>
consteval unsigned round_up_pow2_iters() noexcept
{
	constexpr int bits = std::numeric_limits<T>::digits;
	static_assert(bits > 0);
	
	unsigned int result = 1;

	return log2_of_rounded_up_to_pow2(bits);
}

template<class T>
constexpr T round_up_pow2(T x)
{
	unsigned iters = round_up_pow2_iters<T>();
	int shift = 1;

	--x;
	while (iters --> 0)
		x |= x >> shift;
	++x;
	return x;
}

template<std::integral T>
T estimate_babylonial(T x)
{
	return T(1) << (1 + std::bit_width((std::make_unsigned_t<T>)x) / 2);
}
template<class T>
T estimate_babylonial(const T& x)
{
	return x / 2;
}

template<class T>
constexpr T isqrt1(const T& n)
{
	if (n <= 1)
		return n;

	T current = estimate_babylonial(n);
	T next = (current + n / current) / 2;
	while (next < current)
	{
		current = next;
		next = (current + n / current) / 2;
	}
	return current;
}


template<class T> requires(std::numeric_limits<T>::is_specialized && std::numeric_limits<T>::is_bounded)
constexpr T estimate_max_sqrt()
{
	return isqrt1(std::numeric_limits<T>::max());
}
template<class T> requires(std::numeric_limits<T>::is_specialized && !std::numeric_limits<T>::is_bounded)
bool a2_exceeds_b(const T& a, const T& b)
{
	return a * a > b;
}
template<class T>
bool a2_exceeds_b(const T& a, const T& b)
{
	//overflow prevention
	if (a > estimate_max_sqrt<T>())
		return true;
	return a * a > b;
}
template<std::integral T>
T isqrt2(const T& n)
{
	if (!n)
		return n;
	T l = 1, r = n;
	while (l != r)
	{
		T m = (l + r + 1) / 2;
		if (a2_exceeds_b(m, n))
			r = m - 1;
		else
			l = m;
	}

	return l;
}
template<std::integral T>
T isqrt(T n)
{
	return isqrt1(n);
}

template <class T>
T ferma(const T& n)
{
	int32_t
		x = isqrt1(n),
		y = 0,
		r = x * x - y * y - n;
	for (;;)
		if (r == 0)
			return x != y ? x - y : x + y;
		else
			if (r > 0)
			{
				r -= y + y + 1;
				++y;
			}
			else
			{
				r += x + x + 1;
				++x;
			}
}


template<class T, class Iterator>
void factorize(T x, Iterator factors, const T& min = {})
{
	T up = isqrt(x);
	for (T i = 2; i <= up;)
	{
		if ((x % i) == 0)
		{
			up = isqrt(x /= i);
			if (i >= min)
			{
				*factors = i;
				++factors;
			}
		}
		else
			++i;
	}
	if (x > 1 && x >= min)
		*factors = x;
}

template<class T>
bool is_prime(T x)
{
	T up = isqrt(x);
	for (T i = 2; i <= up; ++i)
		if ((x % i) == 0)
			return false;
	return true;
}


static std::mt19937_64 _prime_generator_engine;

template<class T>
class prime_generator
{
	std::uniform_int_distribution<T> dist;
	std::deque<T> primes;

	static constexpr T maxT()
	{
		return std::numeric_limits<T>::max();
	}

	void get_primes()
	{
		//no primes? o_0
		factorize(a(this->dist(_prime_generator_engine)), std::back_inserter(this->primes), this->dist.min());
	}

	static T a(const T& x) noexcept
	{
		return 6 * x + 1;
	}
	static T b(const T& x) noexcept
	{
		return (x - 1) / 6;
	}

public:
	prime_generator(T min, T max) noexcept
		: dist(b(min), b(max))
	{
	}
	
	T operator()() noexcept
	{
		while (primes.empty())
			this->get_primes();
		T x = std::move(this->primes.front());
		this->primes.pop_front();
		return x; //RVO pretty please?
	}
};

void foo1()
{
	uint32_t a = INT_MAX;
	uint32_t d = ferma(a);
	uint32_t q = a / d;
	uint32_t r = a % d;
	std::cout << d << ' ' << q << ' ' <<  r;
}

int main()
{
	foo1();

	prime_generator<uint64_t> g(1ull << 50, 1ull << 51);
	std::ofstream null("/dev/null");
	null << 0;

	while (true)
	{
		auto clock = std::chrono::steady_clock::now;
		auto t1 = clock();
		null << g();
		auto t2 = clock();
		auto dt = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		std::cout << dt << '\n';
	}
}







template<class T>
T replace_if_equal(T x, const T& to, T with)
{
	if (x == to)
		return with;
	return x;
}

template<std::random_access_iterator Iter>
void xrotate(Iter first, Iter new_first, Iter end)
{
	const size_t n = end - first;
	const size_t k = replace_if_equal<size_t>(new_first - first, n, 0);
	std::span _debug(first, n);

	if (n <= 1 || k == 0)
		return;

	for (size_t i = 0; i < n - k; ++i)
		iter_swap(first + i, first + i + k);
	return xrotate(first + n - k, first + n - n % k, end);
}

int main1()
{
	std::vector<int> v{ 0,1,2,3,4,5,6,7,8,9 };
	xrotate(v.begin(), v.begin() + 9, v.end());
	return 0;
}
