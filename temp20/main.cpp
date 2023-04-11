
import <vector>;
import <ranges>;
import <algorithm>;
import <numeric>;
import <random>;
import <iostream>;
import <format>;
import libksn.math;

template<class Rng>
	requires(std::ranges::forward_range<Rng>)
double wilcoxon_rank_sum_test_sorted(const Rng& a, const Rng& b)
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

	const size_t m = std::ranges::distance(a);
	const size_t n = std::ranges::distance(b);
	const size_t W = (size_t)std::ceil(sum);

	ksn::wilcoxon_Udistribution distribution(m, n);
	return 1 - distribution.cdf(W - m * (m + 1) / 2);
}

template<std::ranges::random_access_range Rng>
double wilcoxon_rank_sum_test(Rng& a, Rng& b)
{
	std::ranges::sort(a);
	std::ranges::sort(b);
	return wilcoxon_rank_sum_test_sorted(a, b);
}


int main()
{
	std::mt19937_64 rng(0);
	std::uniform_int_distribution<int> distr(0, 1000);

	std::vector<int> a, b;
	const size_t N = 100;
	for (int i = 0; i < N; ++i)
	{
		a.push_back(distr(rng));
		b.push_back(distr(rng));
	}

	auto p = wilcoxon_rank_sum_test(a, b);
	std::cout << std::format("p-value = {}\n", p);
}
