#include <math.h>
#include <stdio.h>

#include <numeric>
#include <utility>
#include <memory>

double foo(double x, void*)
{
	return sin(x);
}

template<class fp_t, class callable_t, class allocator_t = std::allocator<fp_t>, class... params_t>
fp_t differentiate(fp_t&& x, callable_t&& foo, size_t order = 1, allocator_t&& allocator = {}, params_t&& ...params)
{
#define y(x) foo(x, std::forward<params_t>(params)...)
	
	using std::numeric_limits;

	if (order == 0)
	{
		return y(x);
	}
	
	const fp_t epsilon = numeric_limits<fp_t>;

	if (order == 1)
	{
		fp_t x1 = (x == 0) ? epsilon : (x * (epsilon + 1));
		if (x == 0)
		{
			return (y(x1) - y(x)) / epsilon;
		}
		else
		{
			return (y(x1) - y(x)) / (x * epsilon);
		}
	}

	fp_t dx = (x == 0) ? epsilon : x * epsilon;
	fp_t* storage = std::allocator_traits<allocator_t>::allocate(allocator, order + 1);

	std::allocator_traits<allocator_t>::construct(allocator, storage, y(x));
	{
		fp_t x1(x);
		fp_t* p = storage;
		for (size_t i = 1; i <= order; ++i)
		{
			x1 += dx;
			++p;
			std::allocator_traits<allocator_t>::construct(allocator, p, y(x1));
		}
	}

	for (size_t i = order; i != 0; --i)
	{
		for (size_t j = 0; j < order; ++j)
		{
			storage[j] = (storage[j + 1] - storage[j]) / dx;
		}
	}

	if constexpr (!std::is_trivially_destructible_v<fp_t>)
	{
		for (size_t i = 0; i <= order; ++i)
		{
			std::allocator_traits<allocator_t>::destroy(allocator, storage + i);
		}
	}

	dx = storage[0];
	std::allocator_traits<allocator_t>::deallocate(allocator, storage, order + 1);

	return dx;

#undef y
}

template<class fp_t, class callable_t, class... params_t>
fp_t integrate(fp_t a, fp_t b, fp_t (*foo)(fp_t, params_t...), bool recalculate_diff = false, fp_t prec = 1e-6, params_t&& ...params)
{
	if (a == 0 && b == 0) return 0;

	using std::abs;
	const fp_t range = b - a;
	fp_t sum = 0;
	fp_t multiplier = 1;
	fp_t diff, temp;

	size_t i = 1;
	if (abs(b) > abs(a))
	{
		a /= b;
		b = 1;
	}
	else
	{
		b /= a;
		a = 1;
	}

	size_t i = 0;
	do
	{
		diff = differentiate(a, foo, i, {}, std::forward<params_t>(params)...);
		multiplier *= range / (++i);
		diff *= multiplier;
		if (recalculate_diff)
		{
			temp = sum + diff;
			diff = temp - diff;
			sum = temp;
		}
		else
		{
			sum += multiplier * diff;
		}
	} 	while (diff > prec);
}

int main()
{
	double a = std::numeric_limits<double>::digits10;
	for (int i = 0; i < 7; ++i)
	{
		//a = differentiate(0.5, [](double x) -> double { return x * x * x; }, i);
	}

}