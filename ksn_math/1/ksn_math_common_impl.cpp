// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <ksn/math_common.hpp>
#include <stdexcept>



#ifdef _KSN_COMPILER_MSVC
#pragma warning(disable : 26451)
#endif


_KSN_BEGIN



_KSN_DETAIL_BEGIN

#define repeat1(x) (x)
#define repeat2(x) repeat1(x), repeat1(x)
#define repeat4(x) repeat2(x), repeat2(x)
#define repeat8(x) repeat4(x), repeat4(x)
#define repeat16(x) repeat8(x), repeat8(x)
#define repeat32(x) repeat16(x), repeat16(x)
#define repeat64(x) repeat32(x), repeat32(x)
#define repeat128(x) repeat64(x), repeat64(x)

const signed char _log2_lookup_table8[256] =
{
	-1, //some sort of -infinity for log(0)
	repeat1(0), repeat2(1), repeat4(2), repeat8(3),
	repeat16(4), repeat32(5), repeat64(6), repeat128(7)
};

_KSN_DETAIL_END



size_t log2_8(uint8_t x)
{
	return detail::_log2_lookup_table8[x];
}

size_t log2_16(uint16_t x)
{
	uint8_t t;
	return (t = (x >> 8)) ? (8 + detail::_log2_lookup_table8[t]) : (detail::_log2_lookup_table8[x]);
}

size_t log2_32(uint32_t x)
{
	uint16_t t1, t2;

	if ((t2 = (x >> 16)))
	{
		return (t1 = (t2 >> 8)) ? (16 + 8 + detail::_log2_lookup_table8[t1]) : (16 + detail::_log2_lookup_table8[t2]);
	}
	else
	{
		return (t2 = (x >> 8)) ? (8 + detail::_log2_lookup_table8[t2]) : (detail::_log2_lookup_table8[x]);
	}
}





std::vector<double> polynomial_multiplication(const std::vector<double>& v1, const std::vector<double>& v2)
{
	if (v1.size() == 0 || v2.size() == 0)
	{
		return {};
	}
	if (v1.size() == 1)
	{
		std::vector<double> result(v2);
		for (double& x : result)
		{
			x *= v1[0];
		}
		return result;
	}
	if (v2.size() == 1)
	{
		std::vector<double> result(v1);
		for (double& x : result)
		{
			x *= v2[0];
		}
		return result;
	}

	std::vector<double> result(v1.size() + v2.size() - 1, 0);

	for (size_t i = 0; i < v1.size(); i++)
	{
		for (size_t j = 0; j < v2.size(); ++j)
		{
			result[i + j] += v1[i] * v2[j];
		}
	}

	return result;
}

std::vector<double> polynomial_interpolation(const std::vector<std::pair<double, double>>& points)
{
	if (points.size() == 0)
	{
		return {};
	}

	size_t n = points.size();
	std::vector<double> result(n, 0), temp;
	temp.reserve(n);

	for (size_t i = 0; i < n; ++i)
	{
		temp = { 1 };
		double denominator = 1;

		for (size_t j = 0; j < n; ++j)
		{
			if (i == j)
			{
				continue;
			}

			temp = polynomial_multiplication(temp, { -points[j].first, 1 });
			denominator *= points[i].first - points[j].first;
		}

		for (size_t j = 0; j < n; ++j)
		{
			result[j] += points[i].second * temp[j] / denominator;
		}
	}

	return result;
}



uint64_t isqrt(uint64_t n)
{
	if (n == UINT64_MAX)
	{
		return UINT32_MAX;
	}
	uint64_t x = n, y = 1;
	while (x > y)
	{
		x = (x + y) / 2;
		y = n / x;
	}
	return x;
}
uint32_t isqrt(uint32_t n)
{
	if (n == UINT32_MAX)
	{
		return UINT16_MAX;
	}
	uint_fast32_t x = n, y = 1;
	while (x > y)
	{
		x = (x + y) / 2;
		y = n / x;
	}
	return x;
}
uint16_t isqrt(uint16_t n)
{
	if (n == UINT16_MAX)
	{
		return UINT8_MAX;
	}
	uint_fast16_t x = n, y = 1;
	while (x > y)
	{
		x = (x + y) / 2;
		y = n / x;
	}
	return x;
}
uint8_t isqrt(uint8_t n)
{
	if (n == UINT8_MAX)
	{
		return 15;
		//return UINT4_MAX;
	}
	uint_fast8_t x = n, y = 1;
	while (x > y)
	{
		x = (x + y) / 2;
		y = n / x;
	}
	return x;
}



//_KSN_DETAIL_BEGIN


//
//template<>
//constexpr const char* _fp_int_cast_printf_format_helper<uint8_t> = "%hhu %n";
//template<>
//constexpr const char* _fp_int_cast_printf_format_helper<int8_t> = "%hhi %n";
//template<>
//constexpr const char* _fp_int_cast_printf_format_helper<uint16_t> = "%hu %n";
//template<>
//constexpr const char* _fp_int_cast_printf_format_helper<int16_t> = "%hi %n";
//template<>
//constexpr const char* _fp_int_cast_printf_format_helper<uint32_t> = "%u %n";
//template<>
//constexpr const char* _fp_int_cast_printf_format_helper<int32_t> = "%i %n";
//template<>
//constexpr const char* _fp_int_cast_printf_format_helper<uint64_t> = "%llu %n";
//template<>
//constexpr const char* _fp_int_cast_printf_format_helper<int64_t> = "%lli %n";


//_KSN_DETAIL_END



int solve_quadratic(long double a, long double b, long double c, long double(*p_roots)[2])
{
	if (c == 0)
	{
		(*p_roots)[0] = 0;
		(*p_roots)[1] = -b / a;
		return 2;
	}
	c /= a;
	b /= (2 * a);
	long double D = b * b;
	D -= c;
	if ( ::fabsl(D) < ::powl( 10, int(::log10(c > 0 ? c : -c)) - LDBL_DIG + 1 ) ) //Consider this as
	{
		D = 0;
	}
	if (D < 0)
	{
		return 0;
	}
	
	D = ::sqrtl(D);

	(*p_roots)[0] = -b + D;
	(*p_roots)[1] = -b - D;

	return 2;
}

int solve_quadratic(long double a, long double b, long double c, ksn::complex(*p_roots)[2])
{
	ksn::complex D = sqrt(ksn::complex(b * b - 4 * a * c));

	(*p_roots)[0] = (-b + D) / 2;
	(*p_roots)[1] = (-b - D) / 2;

	return 2;
}

int solve_quadratic(const ksn::complex& a, const ksn::complex& b, const ksn::complex& c, ksn::complex(*p_roots)[2])
{
	ksn::complex D = sqrt(ksn::complex(b * b - 4 * a * c));

	(*p_roots)[0] = (-b + D) / 2;
	(*p_roots)[1] = (-b - D) / 2;

	return 2;
}


long double solve_cubic_principal(long double a, long double b, long double c, long double d)
{
	b /= a;
	c /= a;
	d /= a;
	//we don't need variable a anymore, reuse it
	a = -b / 3;
	long double q = a * a * a + b * a * a + c * a + d;
	long double p = 3 * a * a + 2 * b * a + c;
	q /= 2;
	p /= 3;
	p = p * p * p;
	p += q * q;

	if (::fabs(p) < 1e-7) 
	{
		p = 0;
	}

	if (p >= 0)
	{
		p = ::sqrtl(p);
		return ::cbrtl(-q + p) + ::cbrtl(-q - p) + a;
	}
	else
	{
		complex x1, x2;
		x1 = x2 = ksn::sqrt(complex(p));
		x1 = ksn::cbrt(-q + x1);
		x2 = ksn::cbrt(-q - x2);
		complex result = x1;
		x1 += x2;
		x1 += a;
		return x1.real();

		//long double angle = ::atan2l(::sqrtl(-p), -q);
		//long double length;
		//if (::fabsl(::cos(angle)) < 1e-4)
		//{
		//	length = -p / ::sinl(angle);
		//}
		//else
		//{
		//	length = -q / ::cosl(angle);
		//}
		//length = ::powl(length, 1.0 / 3);
		//long double result = 2 * length * ::cosl(angle / 3) + a;

		//if (result < 0)
		//{
		//	result -= 2 * ::powl(10, int(::log10(-result))) * LDBL_EPSILON;
		//}
		//else
		//{
		//	result += 2 * ::powl(10, int(::log10(result))) * LDBL_EPSILON;
		//}
		//return result;
	}
}

int solve_cubic(long double a, long double b, long double c, long double d, long double(*p_roots)[3])
{
	//f(x) = a*x^3 + b*x^2 + c*x + d = 0

	//Let e = -b/(3*a)
	//Let x = y + e

	//Substitute x = y + e into f(x) to get
	//f(x) = a*y^3 + p*y + q = 0
	//Solve using Cardano's formula

	long double e = -b / (3 * a);
	//q = f(e)
	long double q = a * e * e * e + b * e * e + c * e + d, q1 = q;
	//p = f'(e)
	long double p = 3 * a * e * e + 2 * b * e + c;

	q1 /= -2;
	q = q * q / 4;
	p = p * p * p / 27;

	ksn::complex Q = q + p;
	Q = ksn::pow(Q, 1.0 / 2);

	ksn::complex t1, t2;
	t1 = q1 + Q;
	t2 = q1 - Q;

	t1 = ksn::cbrt(t1);
	t2 = ksn::cbrt(t2);

	return 0;
}

int solve_qartic(long double a, long double b, long double c, long double d, long double e, long double(*p_roots)[4])
{
	if (a == 0)
	{
		printf("Coefficient a must be a non-zero value\n");
		return -1;
	}

	if (e == 0)
	{
		(*p_roots)[0] = 0;
		double x = solve_cubic_principal(a, b, c, d);
		(*p_roots)[1] = x;
		return 2 + solve_quadratic(a, b + x * a, c + x * (b + x * a), (long double(*)[2])(*p_roots + 2));
	}

	b /= a;
	c /= a;
	d /= a;
	e /= a;

	long double f = -b / 4;

	long double r = e + f * (d + f * (c + f * (b + f)));
	long double q = d + f * (2 * c + f * (3 * b + f * 4 * a));
	long double p = c + 3 * f * (b + 2 * f);

	long double t = ksn::solve_cubic_principal(1, -p, -4 * r, 4 * p * r - q * q);

	long double omega[2];
	int found_omega = ksn::solve_quadratic(1, -t, r, &omega);
	if (found_omega == 0)
	{
		return 0;
	}

	long double y;
	double delta_w = omega[1] - omega[0];
	if (q == 0)
	{
		if (delta_w == 0)
		{
			y = ::sqrtl(t - p);
			//y += epsilon(y);
		}
		else
		{
			y = 0;
		}
	}
	else if (p <= t)
	{
		y = ::sqrtl(t - p);
		//y += epsilon(y);
		if (delta_w * q < 0)
			y = -y;
	}
	else
	{
		if (omega[0] == omega[1])
		{
			return 0;
		}
		y = q / delta_w;
	}

	int r1, r2;

	r1 = ksn::solve_quadratic(1, y, omega[0], (long double(*)[2])p_roots);
	if (r1)
	{
		r2 = ksn::solve_quadratic(1, -y, omega[1], (long double(*)[2])(*p_roots + 2));
	}
	else
	{
		r2 = ksn::solve_quadratic(1, -y, omega[1], (long double(*)[2]) * p_roots);
	}

	int roots_count = r1 + r2;

	for (int i = 0; i < roots_count; ++i)
	{
		(*p_roots)[i] += f;
	}

	return roots_count;
}



bool compare_equal_with_precision(const complex& a, const complex& b, long double precision)
{
	long double dr = a.data[0] - b.data[0];
	long double di = a.data[1] - b.data[1];

	return dr > -precision && dr < precision && di > -precision && di < precision;
}



long double sign(long double x)
{
	if (x == 0) //-V550
	{
		return 0;
	}
	return x < 0 ? -1 : 1;
}

bool isnan(const ksn::complex& x)
{
	return ::isnan(x.data[0]) || ::isnan(x.data[1]);
}

bool isinf(const ksn::complex& x)
{
	return ::isinf(x.data[0]) || ::isinf(x.data[1]);
}

_KSN_DETAIL_BEGIN

template<>
void _newthon_method_helper_adjust_step(ksn::complex& step, const ksn::complex& x, int64_t k)
{
	step.data[0] = k * LDBL_EPSILON * sign(x.data[0]);
	step.data[1] = k * LDBL_EPSILON * sign(x.data[1]);
}

_KSN_DETAIL_END




static _KSN_FORCEINLINE complex get_initial_point(complex z, int k)
{
	complex two_pi_k_i{ 0, 2 * KSN_PI * k };
	complex ip{ log(z) + two_pi_k_i - log(log(z) + two_pi_k_i) };// initial point coming from the general asymptotic approximation
	complex p{ sqrt(2 * (KSN_E * z + 1)) };// used when we are close to the branch cut around zero and when k=0,-1

	if (abs(z - (-exp(-1))) <= 1) //we are close to the branch cut, the initial point must be chosen carefully
	{
		if (k == 0) ip = -1 + p - 1 / 3 * pow(p, 2) + 11 / 72 * pow(p, 3);
		if (k == 1 && z.imag() < 0) ip = -1 - p - 1 / 3 * pow(p, 2) - 11 / 72 * pow(p, 3);
		if (k == -1 && z.imag() > 0) ip = -1 - p - 1 / 3 * pow(p, 2) - 11 / 72 * pow(p, 3);
	}

	if (k == 0 && abs(z - .5) <= .5) ip = (0.35173371124919582 * (0.1237166 + 7.061302897 * z)) / (2 + 0.827184 * (1 + 2 * z));// (1,1) Pade approximant for W(0,a)

	if (k == -1 && abs(z - .5) <= .5) ip = -((complex(2.259158898533606,
		4.22096096926619669) * (complex(-14.073271, - 33.767687754) * z - complex(12.7127, -
			19.071643) * (1 + 2 * z))) / (2 - complex(17.23103, - 10.629721) * (1 + 2 * z)));// (1,1) Pade approximant for W(-1,a)

	return ip;
}

static _KSN_FORCEINLINE complex zexpz(complex z)
{
	return z * exp(z);
}
static _KSN_FORCEINLINE long double zexpz(long double z)
{
	return z * ::expl(z);
}

//The derivative of z * ::expl(z) = exp(z) + z * exp(z) = e^z * (z + 1)
static _KSN_FORCEINLINE complex zexpz_d(complex z)
{
	return exp(z) + z * exp(z);
}
static _KSN_FORCEINLINE long double zexpz_d(long double z)
{
	return ::expl(z) + z * ::expl(z);
}

//The second derivative of z * exp(z) = 2 * exp (z) + z * exp(z) = e^z * (z + 2)
static _KSN_FORCEINLINE complex zexpz_dd(complex z)
{
	return 2 * exp(z) + z * exp(z);
}
static _KSN_FORCEINLINE long double zexpz_dd(long double z)
{
	return 2 * ::expl(z) + z * ::expl(z);
}

long double lambert_W0(long double x)
{
	if (::fabs(x) <= 3 * LDBL_EPSILON)
	{
		return 0;
	}

	if (x < 0)
	{
		if (x < -1 / KSN_E - 3 * LDBL_EPSILON)
		{
			_KSN_RAISE(std::invalid_argument("LambertW0 belongs to R only when x >= -1/e"));
		}

		if (::fabsl(x - -1 / KSN_E) <= 2 * LDBL_EPSILON)
		{
			return -1;
		}
	}

	return lambert_W(x, 0).data[0];
}

long double lambert_W_n1(long double x)
{
	if (::fabsl(x - 1 / KSN_E) <= 3 * LDBL_EPSILON)
	{
		return -1;
	}

	if (x < -1 / KSN_E || x > 3 * LDBL_EPSILON)
	{
		_KSN_RAISE(std::invalid_argument("LambertW(-1, x) belongs to R only when -1/e <= x <= 0"));
	}

	return lambert_W({ x, 0 }, -1).data[0];
}

complex lambert_W(complex z, int k)
{
	//For some particular z and k W(z,k) has simple value:
	if (z == 0) return (k == 0) ? 0 : -INFINITY;
	if (z == -::expl(-1) && (k == 0 || k == -1)) return -1.;
	if (z == ::expl(1) && k == 0) return 1.;

	//Halley method begins
	complex w{ get_initial_point(z, k) }, wprev{ get_initial_point(z, k) }; // intermediate values in the Halley method
	size_t iter = 0;
	constexpr const size_t maxiter = 30; // max number of iterations. This eliminates improbable infinite loops
	double prec = 1.E-30; // difference threshold between the last two iteration results (or the iter number of iterations is taken)

	do
	{
		wprev = w;
		w -= 2 * ( (zexpz(w) - z) * zexpz_d(w) ) /
			( 2 * pow(zexpz_d(w), 2) - (zexpz(w) - z) * zexpz_dd(w) );
		iter++;
	} while ((abs(w - wprev) > prec) && iter < maxiter);

	return w;
}


_KSN_END
