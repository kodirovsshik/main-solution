
/*
import <ranges>;
import <algorithm>;

template<class Rng>
	requires(std::ranges::forward_range<Rng>)
bool wilcoxon_rank_sum_test_sorted(const Rng& a, const Rng& b, double significance = 0.05)
{
	if (std::ranges::distance(a) > std::ranges::distance(b))
		return wilcoxon_rank_sum_test_sorted(b, a);

	auto pa = a.begin();
	auto pb = b.begin();
	const auto pae = a.end();
	const auto pbe = b.end();

	if (pa == pae || pb == pbe)
		return false;

	auto iter_with_min = []
	(auto&& It1, auto&& It2) -> auto&&
	{
		if (*It2 < *It1)
			return It2;
		else
			return It1;
	};

	auto last_inserted = [&]
	{
		if (*pa < *pb)
			return pa++;
		else
			return pb++;
	}();

	size_t first_rank = 1;
	size_t last_rank = 1;
	size_t same_run_size = pb != std::ranges::begin(b);
	double sum = 0;

	auto accumulate_and_flush_rank_range = [&]
	{
		double mid = (first_rank + last_rank) * 0.5;
		sum += mid * same_run_size;
		++last_rank;
		first_rank = last_rank;
		same_run_size = 0;
	};

	auto iteration = [&]
	(auto& next)
	{
		if (*next > *last_inserted)
			accumulate_and_flush_rank_range();
		else
			++last_rank;

		last_inserted = next;
		same_run_size += (&next == &pb);
		++next;
	};
	
	while (true)
	{
		if (pa == pae || pb == pbe)
			break;

		auto& next = iter_with_min(pa, pb);
		iteration(next);
	}

	while (pb != pbe)
		iteration(pb);

	accumulate_and_flush_rank_range();

	return false;
}

template<std::ranges::random_access_range Rng>
bool wilcoxon_rank_sum_test(Rng& a, Rng& b, double significance = 0.05)
{
	std::ranges::sort(a);
	std::ranges::sort(b);
	return wilcoxon_rank_sum_test_sorted(a, b, significance);
}
*/

/*
import <fstream>;
import <vector>;
import <cstddef>;

static constexpr size_t
	n = 15,
	m = 15;

size_t N = 0;

std::ofstream fout("a.txt");
std::vector<int> c((n + m)* (n + m + 1) / 2 + 1, 0);
*/

/*
import <vector>;
import <numeric>;

template<class Callable>
	requires (std::invocable<Callable, const std::vector<size_t>&>)
void enumerate_subsets(size_t n, size_t k, Callable&& f)
{
	if (k == 0)
		return;

	std::vector<size_t> idx(k);
	std::ranges::iota(idx, 0);

	while (true)
	{
		if (idx.back() == n)
			break;

		std::invoke(f, idx);

		size_t i = 0;
		while (true)
		{
			++idx[i];
			if (i == k - 1)
				break;

			if (idx[i] == idx[i + 1])
				idx[i] = i;
			else
				break;
			++i;
		}
	}
}


void payload_distribution_calculator()
{
	enumerate_subsets(n + m, m, []
	(const std::vector<size_t>& nums)
		{
			size_t sum = 0;
			for (auto x : nums)
				sum += x;
			sum += m;
			++c[sum];
			++N;
		}
	);
}
*/



#include <multivector.hpp>
#include <iostream>
#include <format>


using T = double;
multivector<T, 3> v;

constexpr size_t n = 100;
constexpr size_t m = 100;
constexpr size_t w = n + m;
constexpr int64_t S = 5100;

auto arr_ref = [&]
(int64_t S, int64_t m, int64_t w) -> double&
{
	return v[{(size_t)S, (size_t)m, (size_t)w}];
};
auto arr_val = [&]
(int64_t S, int64_t m, int64_t w) -> double
{
	if (S < 0 || m < 0 || w < 0)
		return 0;
	return arr_ref(S, m, w);
};

double P(int64_t S, int64_t m, int64_t w)
{
	auto val = arr_val(S, m, w);
	if (val != val)
		val = arr_ref(S, m, w) = P(S, m, w - 1) + P(S - w, m - 1, w - 1);
	return val;
};

double inv_nck(size_t n, size_t k)
{
	k = std::min(k, n - k);
	size_t top = 1;
	size_t bottom = n - k + 1;

	double x = 1;
	while (top <= k)
	{
		x *= top++;
		x /= bottom++;
	}
	return x;
}

int main()
{
	double inverse_sums_count = inv_nck(w, m);
	
	v.resize({ S+1, m+1, w+1 });
	for (auto& x : v)
		x = NAN;
	for (size_t i = 0; i <= w; ++i)
		v[{0, 0, i}] = inverse_sums_count;

	for (int s = 5050; s < 5100; ++s)
		std::cout << std::format("{:.16e}\n", P(s, m, w));

	size_t x1 = v.size();
	size_t x2 = 0;
	for (auto x : v)
		x2 += (x == x);

	std::cout << std::format("{}/{} ({:.3}%)\n", x2, x1, (double)x2 / x1 * 100);
}
