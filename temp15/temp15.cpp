//#include <complex>
#include <ksn/math_complex.hpp>

#pragma warning(disable : 4996)

using fp = double;
using cfp = ksn::complex<fp>;

cfp I = cfp(0, 1);

cfp f(cfp z)
{
	using std::exp;
	using std::sqrt;
	return exp(I * sqrt(z)) / (z + 1);
}

int main()
{
	constexpr size_t N = 1000;
	constexpr cfp z = -1;
	constexpr fp eps = 0.000000000001;
	const cfp rotator = cfp(cos(2 * KSN_PI / N), sin(2 * KSN_PI / N));
	
	cfp sum = 0;
	cfp disp = eps;
	cfp prev = f(z + disp);

	for (size_t n = 0; n < N; ++n)
	{
		cfp dz = disp * (rotator - 1);
		disp *= rotator;

		auto y = f(z + disp);

		sum += (y + prev) / 2 * dz;
		prev = y;
	}
	printf("%lg + i * %lg\n", sum.real, sum.imag);
}
