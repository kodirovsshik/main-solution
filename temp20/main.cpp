
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

	//TODO: calculate W-statistic and p-value
	return false;
}

template<std::ranges::random_access_range Rng>
bool wilcoxon_rank_sum_test(Rng& a, Rng& b, double significance = 0.05)
{
	std::ranges::sort(a);
	std::ranges::sort(b);
	return wilcoxon_rank_sum_test_sorted(a, b, significance);
}


int main()
{
	
}
