
#include <utility>
#include <string_view>
#include <random>
#include <iostream>
#include <fstream>
#include <tuple>
#include <ranges>
#include <algorithm>

template<std::integral T>
T randint(T min, T max)
{
	static constexpr size_t seed = 0;
	static std::mt19937_64 engine(seed);
	std::uniform_int_distribution<T> dist(min, max);
	return dist(engine);
}
template<std::integral T>
T randint(T max)
{
	return randint<T>(0, max);
}

struct string_hasher
{
	size_t a, b, start;

	size_t operator()(std::string_view v)
	{
		size_t hash = start;
		for (char c : v)
			hash = hash * a + c * b;
		return hash;
	}
};

constexpr size_t primes[] = {
	2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97,
	101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199,
	211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293,
	307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397,
	401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499,
	503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599,
	601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691,
	701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797,
	809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887,
	907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997,
};

constexpr size_t n_primes = std::size(primes);

using triple = std::tuple<size_t, size_t, size_t>;

template<size_t N>
size_t getter(const triple& t)
{
	return std::get<N>(t);
}

int main()
{
	std::string_view raw[] =
	{
		{"hello"},
		{"hi"},
		{"aboba"},
		{"a, boba"},
		{"L"},
		{"ratio"},
		{"get real"},
		{"touch grass"},
		{"sup"},
	};
	constexpr size_t N = std::size(raw);

	string_hasher hasher{};
	size_t collision_numbers[N + 1]{};
	size_t tries = n_primes * n_primes * n_primes * 2;

	std::ofstream fout("0C configs.txt");
	std::vector<triple> data;

	do
	{
		hasher.a = primes[randint(n_primes - 1)];
		hasher.b = primes[randint(n_primes - 1)];
		hasher.start = primes[randint(n_primes - 1)];

		size_t collisions = 0;
		size_t table[N]{};
		for (auto&& key : raw)
		{
			if (0 != table[hasher(key) % N]++)
				collisions++;
		}

		collision_numbers[collisions]++;

		if (collisions == 0)
			data.push_back({ hasher.a, hasher.b, hasher.start });

	} while (--tries > 0);

	std::cout << "Collisions: " << std::endl;
	for (size_t i = 0; auto n : collision_numbers)
	{
		std::cout << '[' << i++ << "] = " << n << std::endl;
	}

	std::ranges::sort(data, {}, getter<0>);

	auto current = data.begin();
	const auto end = data.end();
	while (current != end)
	{
		auto top = std::ranges::upper_bound(std::ranges::subrange(current, end), getter<0>(*current), {}, getter<0>);
		std::ranges::sort(std::ranges::subrange(current, top), {}, getter<1>);
		current = top;
	}

	for (auto&& [a, b, c] : data)
		fout << a << ", " << b << ", " << c << '\n';

	return 0;
}
