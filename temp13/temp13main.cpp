
#include <ksn/math_common.hpp>

#include <span>

#pragma warning(disable : 4996)



double polynomial(double x, std::span<double> c)
{
	x = 3.14159265358979323 - x;
	const double x2 = x * x;
	const size_t n = c.size();

	double sum = c[n - 1];;
	for (size_t i = n - 2; i != -1; --i)
		sum = sum * x2 + c[i];
	sum = sum * x2 + 1;
	return sum * x;
}

template<class X, class callee_t>
void ascend_local(callee_t&& f, X& x)
{
	using Y = decltype(f(x));
	static constexpr double twopi = 2 * 3.14159265358979323;

	for (size_t i = 0; i < 20; ++i)
	{
		if (x < 0)
			x = 0;
		if (x > twopi)
			x = twopi;
		Y der = ksn::differentiate1(f, x);
		if (fabs(der) < 0.01)
			x += 0.1;
		else if (fabs(der) < 10)
		{
			double dx = 0.2 * der;
			if (fabs(dx) > 0.2)
				dx = copysign(0.2, dx);
			x += dx;
		}
		else
			break;
	}
}

double max_err2(std::span<double> c)
{
	static constexpr double twopi = 2 * 3.14159265358979323;
	static constexpr size_t N = 50;

	double max = 0;
	auto func = [&]
	(double x)
	{
		double diff = sin(x) - polynomial(x, c);
		return diff * diff;
	};

	ksn::newton_method_params<double> params;
	params.x_min = 0;
	params.x_max = twopi;

	for (double x = 0; x <= twopi; x += twopi / (N - 1))
	{
		double x0 = x;
		ascend_local(func, x0);
		double diff = func(x0);
		if (diff > max)
			max = diff;
	}
	return max;
}

double dedf(size_t i, std::span<double> c)
{
	double x = c[i];
	double dx = 0.000001;
	if (x)
		dx *= x;
	dx = (x + dx) - x;

	c[i] = x + dx;
	double f1 = max_err2(c);

	c[i] = x - dx;
	double f2 = max_err2(c);

	c[i] = x;
	return (f1 - f2) / (2 * dx);
}

int main()
{
	static constexpr size_t N = 4;
	double x[] = { -0.1600294373864122, 0.0054237595389689, 0.0001867560427660, -0.0000133552223974 };
	double grad[] = { 0, 0, 0, 0 };
	const double descend_speed = 0.000000000001;

	FILE* f = fopen("report.txt", "ab");
	if (!f) return -1;

	auto report = [&]
	(FILE* f)
	{
		fprintf(f, "Gradient descend report:\n");
		for (size_t i = 0; i < N; ++i)
			fprintf(f, "C%zu = % 19.16lf\n", i, x[i]);
		fprintf(f, "Max error: %lf\n", sqrt(max_err2(x)));
		for (size_t i = 0; i < N; ++i)
			fprintf(f, "dE/dC%zu = % 19.16lf\n", i, grad[i]);
		fprintf(f, "\n");
	};

	for (size_t iteration = 0; true; ++iteration)
	{
		for (size_t i = 0; i < N; ++i)
			grad[i] = 1. / dedf(i, x);

		if ((iteration % 1000) == 0)
		{
			report(stdout);
			report(f);
		}

		for (size_t i = 0; i < N; ++i)
			x[i] -= descend_speed * grad[i];
	}

	return 0;
}