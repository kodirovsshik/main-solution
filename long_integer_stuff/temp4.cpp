#include <ksn/math_common.hpp>
#include <ksn/math_complex.hpp>
#include <ksn/math_long_integer.hpp>

#include <stdio.h>

#include <intrin.h>



#pragma comment(lib, "ksn_math.lib")
#pragma comment(lib, "ksn_debug_tools.lib")

#pragma warning(disable : 4996)



void print_polynomial(const std::vector<double>& polynomial, FILE* fd = stdout)
{
	int printed = 0;
	bool did_print = false;

	size_t n = polynomial.size();
	if (n)
	{
		double x = polynomial[n - 1];
		bool do_print_power = true;

		if (abs(x) < 1e-15)
		{
			x = 0;
		}
		if (x == 1)
		{
			printf("x");
			did_print = true;
		}
		else if (x == -1)
		{
			printf("-x");
			did_print = true;
		}
		else if (x == 0)
		{
			do_print_power = false;
		}
		else
		{
			printf("%lgx", x);
			did_print = true;
		}

		if (do_print_power)
		{
			if (n != 2)
			{
				printf("^%zu", n - 1);
			}
			did_print = true;
		}
	}
	if (n > 2)
	{
		for (size_t i = polynomial.size() - 2; i > 0; --i)
		{
			double x = polynomial[i];
			bool do_print_power = true;
			if (abs(x) < 1e-15)
			{
				x = 0;
			}
			if (x == 1)
			{
				printf("+x");
				did_print = true;
			}
			else if (x == -1)
			{
				printf("-x");
				did_print = true;
			}
			else if (x == 0)
			{
				do_print_power = false;
			}
			else
			{
				printf("%+lgx", x);
				did_print = true;
			}

			if (do_print_power)
			{
				if (i != 1)
				{
					printf("^%zu", i);
				}
			}
		}
	}

	double x = polynomial[0];
	if (abs(x) < 1e-15)
	{
		x = 0;
	}

	if (!did_print)
	{
		printf("%lg", x);
	}
	else if (n > 1 && x)
	{
		printf("%+lg", x);
	}
}



typedef std::complex<long double> base;

size_t rev(size_t num, size_t lg_n)
{
	size_t res = 0;
	for (size_t i = 0; i < lg_n; ++i)
	{
		if (num & ((size_t)1 << i))
		{
			res |= (size_t)1 << (lg_n - i - 1);
		}
	}

	return res;
}

void fft(std::vector<base>& a, bool invert)
{
	size_t n = a.size();
	size_t lg_n = 0;
	while ((size_t(1) << lg_n) < n)  ++lg_n;

	for (size_t i = 0; i < n; ++i)
	{
		if (i < rev(i, lg_n))
		{
			swap(a[i], a[rev(i, lg_n)]);
		}
	}

	for (size_t len = 2; len <= n; len <<= 1)
	{
		double ang = -2 * 3.141592653589793 / len * (invert ? -1 : 1);
		base wlen(cos(ang), sin(ang));
		for (size_t i = 0; i < n; i += len)
		{
			base w(1);
			for (size_t j = 0; j < len / 2; ++j)
			{
				base u = a[i + j], v = a[i + j + len / 2] * w;
				a[i + j] = u + v;
				a[i + j + len / 2] = u - v;
				w *= wlen;
			}
		}
	}

	if (invert)
	{
		for (size_t i = 0; i < n; ++i)
		{
			a[i] /= (long double)n;
		}
	}
}

void multiply(const std::vector<int>& a, const std::vector<int>& b, std::vector<int>& res) 
{
	static const constexpr auto max = [](size_t a, size_t b) -> size_t { return a > b ? a : b; };

	std::vector<base> fa(a.begin(), a.end()), fb(b.begin(), b.end());
	size_t n = 1;
	while (n < max(a.size(), b.size()))  n <<= 1;
	n <<= 1;
	fa.resize(n), fb.resize(n);

	fft(fa, false), fft(fb, false);
	for (size_t i = 0; i < n; ++i)
		fa[i] *= fb[i];
	fft(fa, true);

res.resize(n);
for (size_t i = 0; i < n; ++i)
	res[i] = int(fa[i].real() + 0.5);
}

#include <ksn/function.hpp>

int binpow(int a, int n)
{
	int res = 1;

	while (n)
	{
		if (n & 1)
		{
			res *= a;
		}

		a *= a;
		n >>= 1;
	}

	return res;
}

_KSN_BEGIN

//linteger pow(linteger x, size_t n)
//{
//	linteger result(1, x.m_capacity);
//	result.g_capacity_flags = x.g_capacity_flags;
//
//	while (n)
//	{
//		if (n & 1)
//		{
//			result *= x;
//		}
//
//		x *= x;
//		n >>= 1;
//	}
//
//	return result;
//}

_KSN_DETAIL_BEGIN

extern const signed char* _log2_lookup_table8;

_KSN_DETAIL_END

_KSN_END

#include <map>
#include <unordered_map>

#include <vector>

int main(int argc, char** argv)
{



	(void)getchar();
	return 0;
}
