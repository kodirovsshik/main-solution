
#include <ksn/math_complex.hpp>

#include <iostream>

struct identity
{
	template<class T>
	T&& operator()(T&& x) const noexcept
	{
		return std::forward<T>(x);
	}
};

template<class _fp_t, class Callable, class Contour = identity>
auto finite_integrator(const _fp_t& t1, const _fp_t& t2, Callable&& f, Contour&& z = {}, const _fp_t& desired_dt = 0.0001)
{
	using fp_t = std::remove_cvref_t<_fp_t>;
	using fpz_t = std::remove_cvref_t<std::invoke_result_t<Contour, fp_t>>;
	using fps_t = std::remove_cvref_t<std::invoke_result_t<Callable, fpz_t>>;
	using std::fabs;

	const static fp_t three = fp_t(3);

	const fp_t delta = fabs(t2 - t1);
	uint32_t n = (uint32_t)(std::min<fp_t>(delta / desired_dt, UINT32_MAX - 1) + (fp_t)0.5);
	const fp_t dt = delta / n;

	const fp_t dt_mid1 = dt / fp_t(3);
	const fp_t dt_mid2 = fp_t(2) * dt_mid1;
	const fp_t dt_over8 = dt / fp_t(8);

	fp_t t = t1;
	fpz_t z1 = z(t1);
	fps_t sum = 0, f1 = f(z1);
	while (true)
	{
		fps_t fmid1 = f(z(t + dt_mid1));
		fps_t fmid2 = f(z(t + dt_mid2));
		fpz_t z2 = z(t += dt);
		fps_t f2 = f(z2);

		sum += dt_over8 * (f1 + three * (fmid1 + fmid2) + f2);

		if (--n == 0)
			break;
		z1 = z2;
		f1 = f2;
	}

	return sum;
}

int main()
{
	using fpt_t = double;
	using fpz_t = ksn::complex<double>;
	using fps_t = double;

	static constexpr fpt_t pi = (fpt_t)3.14159265358979323846264338L;
	static constexpr fpt_t twopi = (fpt_t)2 * pi;

	double R = 0;
	auto arc = [&]
	(fpt_t t) -> fpz_t
	{
		using std::exp;
		return R * fpz_t(cos(t), sin(t));
	};

	auto f = []
	(fpz_t z)
	{
		auto z2 = z * z;
		return exp(-z2) / (1 + z2);
	};

	fpt_t t1 = 0, t2 = pi / 2;
	for (size_t i = 0; i < 1000; ++i)
	{
		double dr = 10;
		auto s = finite_integrator<fpt_t>(t1, t2, f, arc);
		std::cout << R << ' ' << s.real << " + i * " << s.imag << '\n';
		R += dr;
	}
}
