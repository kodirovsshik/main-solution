
#include <ksn/math_constexpr.hpp>

#include <cmath>


#pragma warning(disable : 26451 4996)

_KSN_BEGIN

template<class fp_t, class callable>
constexpr fp_t integrate_trapezoidal(callable f, fp_t dx)
{
	fp_t sum = 0;
	fp_t current;

	constexpr static fp_t one = 1;
	fp_t x = -1 + dx;
	fp_t prev = f(-1);

	while (x < one)
	{
		current = f(x);
		sum += (prev + current) / 2 * dx;
		prev = current;
		x += dx;
	}

	return sum;
}

template<class fp_t, class callable>
constexpr fp_t integrate_simpsons_1_3(callable f, fp_t dx)
{
	fp_t sum = 0;
	fp_t current;

	constexpr static fp_t one = 1;
	fp_t x = -1 + dx;
	fp_t prev = f(-1);

	while (x < one)
	{
		current = f(x);
		sum += (prev + 4 * f(x - dx / 2) + current) / 6 * dx;
		prev = current;
		x += dx;
	}

	return sum;
}

template<class fp_t, class callable>
constexpr fp_t integrate_simpsons_3_8(callable f, fp_t dx)
{
	fp_t sum = 0;
	fp_t current;

	constexpr static fp_t one = 1;
	fp_t x = -1 + dx;
	fp_t prev = f(-1);

	while (x < one)
	{
		current = f(x);
		sum += (prev + 3 * (f(x - dx / 3 - dx / 3) + f(x - dx / 3)) + current) / 8 * dx;
		prev = current;
		x += dx;
	}
	return sum;
}

template<class fp_t, class callable>
constexpr fp_t integrate(callable f, fp_t x1, fp_t x2, fp_t dx = 0.0001)
{
	if (x1 != x1 || x2 != x2 || dx != dx) return fp_t(NAN);
	if (x1 > x2) return -integrate(f, x2, x1);
	if (dx <= 0) dx = 0.0001;
	if (abs(x1 - x2) <= dx || x1 == x2) return 0;
	if (x1 == -INFINITY && x2 == INFINITY)
	{
		//TODO
	}
	if (x1 == -INFINITY)
	{
		//TODO

	}
	if (x2 == INFINITY)
	{
		//TODO

	}
}

_KSN_END


//#include <ksn/math_pplf.hpp>


int main()
{
	double x = 1ui64 << 63;

	uint64_t i = x;
}
