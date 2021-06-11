#include <stdio.h>
#include <stdlib.h>
#include <new>

//#pragma comment(lib, "D:\\_need\\SFML-2.4.2x64\\lib\\sfml-system-s-d.lib")
//#pragma comment(lib, "D:\\_need\\SFML-2.4.2x64\\lib\\sfml-graphics-s-d.lib")
//#pragma comment(lib, "D:\\_need\\SFML-2.4.2x64\\lib\\sfml-window-s-d.lib")
//wtf why are all the default libs are not in linked in the project settings
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "winmm.lib")

#include <vector>
#include <cmath>

#include <ksn/math_constants.hpp>

//Returns the N roots of N'th degree real-valued polynomial (no root order)
template<class fp_t>
std::vector<fp_t> solve_polynomial(std::vector<fp_t> coeffs)
{
	for (size_t i = 0; i < coeffs.size(); ++i)
	{
		if (coeffs[coeffs.size() - i - 1] == 0)
			coeffs.pop_back();
		else
			break;
	}

	using std::sqrt;
	using std::cbrt;
	using std::cos;
	using std::acos;
	using std::pow;
	
	if (coeffs.size() <= 1) return {};
	if (coeffs.size() == 2) return { -coeffs[0] / coeffs[1] };
	if (coeffs.size() == 3)
	{
		//Quadratic
		fp_t a = std::move(coeffs[2]);
		fp_t b = std::move(coeffs[1]);
		fp_t c = std::move(coeffs[0]);
		b = b / a / 2;
		c = c / a;

		fp_t D = b * b - c;
		if (c < 0) return {};
		D = sqrt(D);
		return { -b - D, -b + D };
	}
	if (coeffs.size() == 4)
	{
		//Cubic
		fp_t a = std::move(coeffs[3]);
		fp_t b = std::move(coeffs[2]);
		fp_t c = std::move(coeffs[1]);
		fp_t d = std::move(coeffs[0]);
		a = 1 / a;
		b *= a;
		c *= a;
		d *= a;

		b /= 3;
		fp_t b1 = -b;

		a = c - 3 * b * b;
		d = b * (2 * b * b - c) + d;

		d /= 2; //q
		a /= 3; //p
		fp_t D = d * d + a * a * a;
		if (D <= 0)
		{
			//Three real roots
			d /= a;
			a = sqrt(-a);
			fp_t k = 2 * a;
			a = 1 / a;
			fp_t t = acos(d * a);
			return
			{
				fp_t(k * cos((t) / 3) + b1),
				fp_t(k * cos((t - 2 * KSN_PI) / 3) + b1),
				fp_t(k * cos((t - 4 * KSN_PI) / 3) + b1)
			};
		}
		else
		{
			d = -d;
			D = sqrt(D);
			return { fp_t(cbrt(d + D) + cbrt(d - D) + b1) };
		}
	}
	if (coeffs.size() == 5)
	{
		//Quartic
		fp_t a = 1 / std::move(coeffs[4]);
		fp_t b = a * std::move(coeffs[3]);
		fp_t c = a * std::move(coeffs[2]);
		fp_t d = a * std::move(coeffs[1]);
		fp_t e = a * std::move(coeffs[0]);

		b /= 2;

		fp_t q = d + b * (b * b - c);
		b /= 2;
		fp_t p = c - 6 * b * b;
		fp_t r = e + b * (-d + b * (c - 3 * b));

		r *= -4;
		b = (r - p * p / 3);
		p /= -3;
		a = (p * (2 * p * p - r) - q * q - 3 * r * p) * -0.5;
		c = a * a + b * b * b;
		if (c < 0)
		{
			c = -c;
			a = 2 * pow(a * a + c, 1.0 / 6) * cos(1.0 / 3 * atan2(sqrt(c), a));
		}
		else
		{
			c = sqrt(c);
			a = cbrt(a + c) + cbrt(a - c);
		}
		a -= p;
		a /= 2;
		d = b * b - r;
		if (d < 0) return {};
		d = sqrt(d);
		b = a + d;
		c = a - d;

		d = p / (c - b);
		e = c + b + 3 * p;
		e = sqrt(e);
		
		double X = 0;
	}
}

int main()
{

	std::vector<double> coeffs = { 1, 4, 6, 4, 1}, roots;
	roots = solve_polynomial(coeffs);

	return 0;
}