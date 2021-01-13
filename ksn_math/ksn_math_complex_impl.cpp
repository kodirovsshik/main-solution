// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <ksn/math_complex.hpp>
#include <ksn/math_common.hpp>

#define real data[0]
#define imag data[1]

#ifdef _KSN_COMPILER_MSVC
#pragma warning(disable : 26451) //Microsoft, Wtf is this? Your compiler yeilds a freaking warning whenever i try to use NAN macro
#endif

_KSN_BEGIN

complex::complex() noexcept :
	data{0, 0}
{
}

complex::complex(long double x) noexcept :
	data{ x, 0 }
{
}

complex::complex(long double _real, long double _imag) noexcept :
	data{ _real, _imag }
{
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26495)

complex::complex(uninitialized_t) noexcept
{
}

#pragma warning(pop)
#endif



complex complex::operator+() const noexcept
{
	return *this;
}
complex complex::operator-() const noexcept
{
	return complex(-this->real, -this->imag);
}



complex operator+(const complex& left, const complex& right) noexcept
{
	complex answer;
	answer.imag = left.imag + right.imag;
	answer.real = left.real + right.real;
	return answer;
}
complex operator-(const complex& left, const complex& right) noexcept
{
	complex answer;
	answer.real = left.real - right.real;
	answer.imag = left.imag - right.imag;
	return answer;
}
complex operator*(const complex& left, const complex& right) noexcept
{
	complex answer;
	answer.real = left.real * right.real - left.imag * right.imag;
	answer.imag = left.real * right.imag + left.imag * right.real;
	return answer;
}
complex operator/(const complex& left, const complex& right) noexcept
{
	double denominator = right.real * right.real + right.imag * right.imag;
	complex answer;
	answer.real = (left.real * right.real + left.imag * right.imag) / denominator;
	answer.imag = (left.imag * right.real - left.real * right.imag) / denominator;
	return answer;
}

complex operator+(const complex& left, long double right) noexcept
{
	return complex(left.real + right, left.imag);
}
complex operator-(const complex& left, long double right) noexcept
{
	return complex(left.real - right, left.imag);
}
complex operator*(const complex& left, long double right) noexcept
{
	return complex(left.real * right, left.imag * right);
}
complex operator/(const complex& left, long double right) noexcept
{
	return complex(left.real / right, left.imag / right);
}

complex operator+(long double left, const complex& right) noexcept
{
	return complex(left + right.real, right.imag);
}
complex operator-(long double left, const complex& right) noexcept
{
	return complex(left - right.real, -right.imag);
}
complex operator*(long double left, const complex& right) noexcept
{
	return complex(left * right.real, left * right.imag);
}
complex operator/(long double left, const complex& right) noexcept
{
	left /= (right.real * right.real + right.imag * right.imag);
	return complex(left * right.real, -left * right.imag);
}



complex& complex::operator+=(const complex& other) noexcept
{
	this->real += other.real;
	this->imag += other.imag;
	return *this;
}
complex& complex::operator-=(const complex& other) noexcept
{
	this->real -= other.real;
	this->imag -= other.imag;
	return *this;
}
complex& complex::operator*=(const complex& other) noexcept
{
	double temp = this->real;
	this->real = this->real * other.real - this->imag * other.imag;
	this->imag = temp * other.imag + this->imag * other.real;
	return *this;
}
complex& complex::operator/=(const complex& other) noexcept
{
	double denominator = other.real * other.real + other.imag * other.imag;
	double temp = this->real;
	this->real = (temp * other.real + this->imag * other.imag) / denominator;
	this->imag = (this->imag * other.real - temp * other.imag) / denominator;
	return *this;
}

complex& complex::operator+=(long double x) noexcept
{
	this->real += x;
	return *this;
}
complex& complex::operator-=(long double x) noexcept
{
	this->real -= x;
	return *this;
}
complex& complex::operator*=(long double x) noexcept
{
	this->real *= x;
	this->imag *= x;
	return *this;
}
complex& complex::operator/=(long double x) noexcept
{
	this->real /= x;
	this->imag /= x;
	return *this;
}



bool complex::operator==(const complex& other) const noexcept
{
	return this->data[0] == other.data[0] && this->data[1] == other.data[1];
}
bool complex::operator!=(const complex& other) const noexcept
{
	return !(this->data[0] == other.data[0] && this->data[1] == other.data[1]);
}

bool complex::operator==(long double x) const noexcept
{
	return this->real == x && this->imag == 0;
}
bool complex::operator!=(long double x) const noexcept
{
	return this->real != x || this->imag != 0;
}

//complex::operator bool() const noexcept
//{
//	return this->real || this->imag;
//}



complex sqrt(const complex& x)
{
	return ksn::root(x, 2);
}
complex cbrt(const complex& x)
{
	return ksn::root(x, 3);
}
complex root(const complex& x, uint64_t k)
{
	if (k == 0)
	{
		return ksn::complex(NAN, NAN);
	}

	complex answer;

	long double angle = ::atan2l(x.imag, x.real);
	long double length = ::powl(x.length(), 1.0l / k);

	int n = int(angle * (k - 1) / (2 * KSN_PI) * (1 + LDBL_EPSILON));
	angle += 2 * KSN_PI * n;
	angle /= k;

	long double sin_v = ::sinl(angle);
	long double cos_v = ::cosl(angle);

	if (sin_v > -5e-16 && sin_v < 5e-16)
	{
		sin_v = 0;
	}
	else if (cos_v > -5e-16 && cos_v < 5e-16)
	{
		cos_v = 0;
	}

	answer.real = length * cos_v;
	answer.imag = length * sin_v;

	return answer;
}
complex pow(const complex& x, long double n)
{
	long double length = ::powl(x.length(), n);
	long double angle = n * ::atan2l(x.imag, x.real);

	complex answer;
	answer.real = length * ::cosl(angle);
	answer.imag = length * ::sinl(angle);
	return answer;
}
complex pow(const complex& x, const complex& n)
{
	//I've spent an hour to derive this fromula just to make it run 1 mcs faster than a^b = e^(b*ln(a))
	//:thumbsup:

	long double result_length, length[2], angle[2], sin_a2, cos_a2, ln_length1, result_angle;

	length[0] = x.length();

	if (length[0] == 0)
		return complex((long double)(n.imag == 0 && n.real == 0), 0);
	//yes i have just picked a boolean and casted it to fp type stfu

	length[1] = ::sqrtl(n.real * n.real + n.imag * n.imag);
	
	angle[0] = ::atan2l(x.imag, x.real);
	angle[1] = ::atan2l(n.imag, n.real);

	sin_a2 = ::sinl(angle[1]);
	cos_a2 = ::cosl(angle[1]);

	ln_length1 = ::logl(length[0]);

	result_length = ::expl(length[1] * (ln_length1 * cos_a2 - angle[0] * sin_a2));

	result_angle = ln_length1 * length[1] * sin_a2 + angle[0] * length[1] * cos_a2;

	complex answer;
	answer.real = result_length * ::cosl(result_angle);
	answer.imag = result_length * ::sinl(result_angle);
	return answer;
}
std::vector<complex> roots(const complex& x, size_t n)
{
	std::vector<complex> result;
	result.reserve(n);

	long double length = ::powl(x.length(), 1.0l / n);
	long double current = atan2(x.imag, x.real) / n;
	long double angle_over_n = 2 * 3.1415926535897932L / n;


	while (n--)
	{
		result.push_back({ length * cos(current), length * sin(current) });
		current += angle_over_n;
	}

	return result;
}

complex log(const complex& x)
{
	return complex(::logl(x.length()), ::atan2l(x.imag, x.real));
}
complex log2(const complex& x)
{
	return complex(::log2l(x.length()), ::atan2l(x.imag, x.real) * ::log2l(KSN_E));
}
complex log10(const complex& x)
{
	return complex(::log10l(x.length()), ::atan2l(x.imag, x.real) * ::log10l(KSN_E));
}
complex logn(const complex& x, long double b)
{
	return log(x) / ::logl(b);
}

complex exp(const complex& x)
{
	long double length = x.length();
	long double angle = ::atan2l(x.imag, x.real);

	long double new_length = ::expl(length * ::cosl(angle));
	long double new_angle = length * ::sinl(angle);

	complex answer;
	answer.real = new_length * ::cosl(new_angle);
	answer.imag = new_length * ::sinl(new_angle);
	return answer;
}

long double abs(const complex& x)
{
	return x.length();
}

long double complex::length() const noexcept
{
	long double arg = this->argument();
	long double sin_v = ::sinl(arg);
	if (::fabsl(sin_v) < 1e-4)
		return this->real / ::cosl(arg);
	else
		return this->imag / sin_v;
}

long double complex::argument() const noexcept
{
	return ::atan2l(this->imag, this->real);
}

#undef real
#undef imag

long double& complex::real() noexcept
{
	return this->data[0];
}
long double& complex::imag() noexcept
{
	return this->data[1];
}

const long double& complex::real() const noexcept
{
	return this->data[0];
}
const long double& complex::imag() const noexcept
{
	return this->data[1];
}


_KSN_END
