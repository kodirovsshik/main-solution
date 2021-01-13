// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <thread>
#include <chrono>
#include <string>
#include <fstream>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <Windows.h>

#include <ksn/math_common.hpp>
#include <ksn/math_complex.hpp>

#include <ksn/stuff.hpp>

#pragma warning(disable : 26451 6293)

//#pragma comment(lib, "ksn_math.lib")
//#pragma comment(lib, "ksn_function.lib")

#include <complex>

#include <iostream>

using namespace std;

#define epsilon(x) ( ( (x) == 0 ? 0 : int(::log10f(::fabs(x))+1) ) * LDBL_EPSILON * sign(x) )

_KSN_BEGIN

std::vector<double> make_polynomial(const int* x, int n)
{
	std::vector<double> coeffs{ 1 };
	for (int i = 0; i < 4; ++i)
	{
		coeffs = ksn::polynomial_multiplication(coeffs, { 1, (double)-x[i] });
	}
	return coeffs;
}




struct float256_t
{
	/*
	Number = (-1)^sign * 10^exponent * mantiss
	72 digits of decimal precision
	:thumbsup:
	*/
private:

	struct _float256_high_bytes
	{
		uint8_t exponent_1;
		uint8_t exponent_2 : 7;
		bool sign : 1;
	};

	struct _float256_exponent
	{
		int16_t exponent : 15;
	};

public:

	//bits 0..239 for mantiss (30 bytes)
	//bits 240..254 for exponent (2 bytes - 1 bit)
	//bit 255 for sign

	uint8_t data[32];



	float256_t() noexcept;
	float256_t(long double) noexcept;
	template<class CharT>
	float256_t(const CharT* p_string) noexcept;

	template<class CharT>
	int parse(const CharT* p_string) noexcept;

	float256_t& operator+=(float256_t other) noexcept;
	float256_t& operator-=(float256_t other) noexcept;
	float256_t& operator*=(const float256_t& other) noexcept;
	float256_t& operator/=(const float256_t& other) noexcept;

	template<class = void>
	_float256_exponent* _pe() const noexcept
	{
		return (_float256_exponent*)((uint8_t*)this + 30);
	}
	template<class = void>
	_float256_high_bytes* _ph() const noexcept
	{
		return (_float256_high_bytes*)((uint8_t*)this + 30);
	}
	template<class = void>
	uint8_t* _pm() const noexcept
	{
		return (uint8_t*)this;
	}
};



template<size_t N> uint8_t arr_add(uint8_t* dst, const uint8_t* src1, const uint8_t* src2)
{
	uint8_t* pe = dst + N;
	int16_t carry = 0;
	while (dst != pe)
	{
		carry = *src1++ + *src2++ + (carry);
		*dst++ = uint8_t(carry);
		carry >>= 8;
	}
	return uint8_t(carry);
}
template<size_t N> uint8_t arr_add(uint8_t* dst, const uint8_t* src, uint8_t x)
{
	uint8_t* pe = dst + N;
	int16_t carry = x;
	while (dst != pe && carry)
	{
		carry = *src++ + carry;
		*dst++ = uint8_t(carry);
		carry >>= 8;
	}
	return uint8_t(carry);
}

template<size_t N> uint8_t arr_sub(uint8_t* dst, const uint8_t* src1, const uint8_t* src2)
{
	uint8_t* pe = dst + N;
	int16_t carry = 0;
	while (dst != pe)
	{
		carry = *src1++ - *src2++ + (carry);
		*dst++ = uint8_t(carry);
		carry >>= 8;
	}
	return uint8_t(carry);
}

template<size_t N> void arr_mul(uint8_t* dst, const uint8_t* src1, const uint8_t* src2)
{
	if (dst == src1 || dst == src2)
	{
		uint8_t result[N];
		arr_mul<N>(result, src1, src2);
		memcpy(dst, result, N * sizeof(uint8_t));
	}
	else
	{
		uint64_t carry = 0;
		for (size_t n = 0; n < N; ++n)
		{
			for (size_t k = 0; k <= n; ++k)
			{
				carry += src1[k] * src2[n - k];
			}
			*dst++ = uint8_t(carry);
			carry >>= 8;
		}
	}
}
template<size_t N> uint8_t arr_mul(uint8_t* dst, const uint8_t* src, uint8_t x)
{
	uint16_t carry = 0;

	for (uint8_t* pe = dst + N; dst != pe;)
	{
		carry = *src++ * x + carry;
		*dst++ = uint8_t(carry);
		carry >>= 8;
	}

	return uint8_t(carry);
}
template<size_t N> void arr_mul_top_bytes(uint8_t* dst, const uint8_t* src1, const uint8_t* src2, size_t max1 = -1, size_t max2 = -1)
{
	bool search_ourself = (max1 & max2) == -1;
	if (search_ourself)
	{
		for (size_t i = N - 1; i != -1; --i)
		{
			if (src1[i])
			{
				max1 = i;
				break;
			}
			if (src2[i])
			{
				max2 = i;
				break;
			}
		}
	}

	//If neither of them contains non-zero byte
	if ((max1 & max2) == -1)
	{
		memset(dst, 0, sizeof(uint8_t) * N);
		return;
	}

	//Search for first's exponent
	if (max1 == -1)
	{
		for (size_t i = search_ourself ? max2 - 1 : N - 1; i != -1; --i)
		{
			if (src1[i])
			{
				max1 = i;
				break;
			}
		}

		//If not found, X*0=0, so return 0
		if (max1 == -1)
		{
			memset(dst, 0, sizeof(uint8_t) * N);
			return;
		}
	}

	//The same logic here
	if (max2 == -1)
	{
		for (size_t i = search_ourself ? max1 : N - 1; i != -1; --i)
		{
			if (src2[i])
			{
				max2 = i;
				break;
			}
		}

		if (max2 == -1)
		{
			memset(dst, 0, sizeof(uint8_t) * N);
			return;
		}
	}

	//If destenation is some of the sources, create a temporary storage
	//and use is as a destenation and then copy it into the actual destenation
	if (dst == src1 || dst == src2)
	{
		uint8_t result[N];
		arr_mul_top_bytes<N>(result, src1, src2, max1, max2);
		memcpy(dst, result, N * sizeof(uint8_t));
		return;
	}

	//If the result fits completely into N base 256 digits, return usual multiplipcatuon

	size_t offset = max1 + max2;
	if (offset < N)
	{
		return arr_mul<N>(dst, src1, src2);
	}

	offset -= N - 1;

	uint64_t carry = 0;

	//Calculate the initial carry
	for (size_t i = 0; i < offset; ++i)
	{
		for (size_t j = 0; j <= i; ++j)
		{
			carry += src1[j] * src2[i - j];
		}
		carry >>= 8;
	}

	for (size_t n = offset; n != N + offset; ++n)
	{
		for (size_t k = n; k <= n; ++k)
		{
			carry += src1[k] * src2[n - k];
		}
		dst[n - offset] = uint8_t(carry);
		carry >>= 8;
	}
}

template<size_t N> void arr_div(uint8_t* quotient, uint8_t* remainder, const uint8_t* dividend, const uint8_t* divisor)
{
	if (remainder == nullptr)
	{
		uint8_t r[N];
		arr_div<N>(quotient, r, dividend, divisor);
		return;
	}

	if (quotient == dividend || quotient == divisor || remainder == dividend || remainder == divisor)
	{
		uint8_t q[N], r[N];
		arr_div<N>(q, r, dividend, divisor);
		memcpy(quotient, q, sizeof(q));
		memcpy(remainder, r, sizeof(r));
		return;
	}

	//Just a long division algo :p

	size_t divisor_max_power = -1;
	for (size_t i = N - 1; i != -1; --i)
	{
		if (divisor[i])
		{
			divisor_max_power = i;
			break;
		}
	}
	if (divisor_max_power == -1)
	{
		//Raise #DE
		int x = 1, y = 0;
		x /= y;

		abort(); //just in case some idiot (just like me) will handle the exception and try to continue execution
	}
	memcpy(remainder, dividend, sizeof(uint8_t) * N);
	memset(quotient + N - divisor_max_power, 0, divisor_max_power * sizeof(uint8_t));
	for (size_t i = N - 1; i != divisor_max_power - 1; --i)
	{
		uint8_t digit = remainder[i] / divisor[divisor_max_power];
		quotient[i - divisor_max_power] = digit;

		int16_t carry = 0;
		const uint8_t* pdiv = divisor, * pe = remainder + i;
		uint8_t* prem = (uint8_t*)pe - divisor_max_power;
		while (prem <= pe)
		{
			carry = *prem - *pdiv++ * digit + carry;
			*prem++ = uint8_t(carry);
			carry >>= 8;
		}
	}
}
template<size_t N> void arr_div(uint8_t* quotient, uint8_t* remainder, const uint8_t* dividend, uint8_t divisor, uint16_t carry = 0)
{
	for (size_t i = N - 1; i != -1; --i)
	{
		carry <<= 8;
		carry += dividend[i];
		quotient[i] = carry / divisor;
		carry %= divisor;
	}
	if (remainder != nullptr)
	{
		*remainder = uint8_t(carry);
	}
}





float256_t::float256_t() noexcept
{
	memset(this, 0, sizeof(*this));
}

float256_t::float256_t(long double x) noexcept
{
	char buffer[64];
	sprintf_s(buffer, "%.*e", LDBL_DIG, x);

	this->float256_t::float256_t(buffer);
}
template<class CharT>

float256_t::float256_t(const CharT* p_string) noexcept
{
	this->parse(p_string);
}



template<class CharT>
int float256_t::parse(const CharT* p_string) noexcept
{
	while (*p_string == ' ')
	{
		p_string++;
	}

	bool negative = false;

	if (*p_string == '-')
	{
		p_string++;
		negative = true;
	}
	else
	{
		negative = false;
	}

	if ((*p_string < '0' || *p_string > '9') && *p_string != '.' && *p_string != 'e' && *p_string != 'E')
	{
		return -1;
	}

	memset(this, 0, sizeof(*this));

	this->_ph()->sign = negative;

	int result = 0;
	uint8_t temp[30];
	int last_zeros = 0;

	const CharT* p_last = p_string, *p_end;
	while (*p_last >= '0' && *p_last <= '9') p_last++;
	p_end = p_last;
	--p_last;
	while (*p_last == '0') p_last--;

	while (p_string <= p_last)
	{
		uint8_t carry = arr_mul<30>(temp, this->_pm(), 10);
		
		if (carry)
		{
			result |= 1;
			break;
		}

		memcpy(this, temp, sizeof(temp));
		arr_add<30>(this->_pm(), this->_pm(), *p_string++ - '0');
	}

	p_string = p_end;

	int exp1 = 0;

	if (result & 1)
	{
		while (isdigit(*p_string)) p_string++;
		if (*p_string == '.')
		{
			while (isdigit(*++p_string));
		}
	}
	else
	{
		if (*p_string == '.')
		{
			++p_string;
			p_last = p_string;

			while (*p_last >= '0' && *p_last <= '9') p_last++;
			p_end = p_last;
			--p_last;
			while (*p_last == '0') p_last--;

			while (p_string <= p_last)
			{
				uint8_t carry = arr_mul<30>(temp, this->_pm(), 10);

				if (carry)
				{
					result |= 1;
					break;
				}

				++exp1;

				memcpy(this, temp, sizeof(temp));
				arr_add<30>(this->_pm(), this->_pm(), *p_string++ - '0');
			}
			p_string = p_end;
		}
	}

	int exp2 = 0;
	if (*p_string == 'e' || *p_string == 'E')
	{
		sscanf_s(p_string + 1, "%d", &exp2);
	}

	exp2 -= exp1;

	if (exp2 > 16383 + 72)
	{
		
	}

	this->_pe()->exponent = exp2;

	return result;
}



uint8_t _precalculated_power_of_ten[73][30];
extern const char* __precalculated_power_of_ten_data;
int __precalculated_power_of_ten_init = []() -> int 
{
	if _KSN_CONSTEXPR_CONDITION(sizeof(char) == sizeof(uint8_t))
	{
		memcpy(_precalculated_power_of_ten, __precalculated_power_of_ten_data, sizeof(_precalculated_power_of_ten));
	}
	else
	{
		for (size_t i = 0; i < sizeof(_precalculated_power_of_ten); ++i)
		{
			_precalculated_power_of_ten[i / 30][i % 30] = uint8_t(__precalculated_power_of_ten_data[i]);
		}
	}

	return 0;
}();


float256_t FLOAT256_MAX = []() -> float256_t 
{
	uint8_t arr[32];
	memset(arr, 0xFF, 32);

	float256_t* result = (float256_t*)&arr;

	result->_ph()->sign = 0;
	result->_pe()->exponent = 16383;

	return *result;
}
();



float256_t& float256_t::operator+=(float256_t x) noexcept
{
	int do_exit = 0;
	if (this->_pe()->exponent == 16383)
	{
		
	}

	const float256_t* px = &x;
	int diff = x._pe()->exponent - this->_pe()->exponent;
	//Adjust the numbers to have the same exponent to perform a*10^n + b*10^n = (a+b)*10^n
	if (diff > 0)
	{
		if (diff > 72)
		{
			return *this = x;
		}
		arr_mul_top_bytes<30>(x._pm(), x._pm(), _precalculated_power_of_ten[diff]);
		x._pe()->exponent = this->_pe()->exponent;
	}
	else if (diff < 0)
	{
		if (diff < -72)
		{
			return *this;
		}
		arr_mul_top_bytes<30>(this->_pm(), this->_pm(), _precalculated_power_of_ten[-diff]);
		this->_pe()->exponent = x._pe()->exponent;
	}

	//The actual addition
	uint8_t carry = arr_add<30>(this->_pm(), this->_pm(), px->_pm()), carry2;

	//Check if we've got 0 at the end of the result
	uint8_t temp[30];
	arr_div<30>(temp, &carry2, this->_pm(), 10, carry);

	//if we can get rid of trailing zero and not cause infinity overflow, do it
	if (!carry2 && this->_pe()->exponent < 16383)
	{
		carry = 0; //We will already get rid of the overflow digit
		do
		{
			memcpy(this->_pm(), temp, sizeof(temp));
			arr_div<30>(temp, &carry2, this->_pm(), 10);
			this->_pe()->exponent++;
		} while (carry2 == 0 && this->_pe()->exponent < 16383);
		//And repeat until we can not
	}

	//If we can not store the top digit, adjust the number so that we can
	if (carry)
	{
		arr_div<30>(this->_pm(), nullptr, this->_pm(), 10, carry);
		this->_pe()->exponent++;
	}

	if (this->_pe()->exponent == 16383)
	{
		memset(this, 0, sizeof(temp));
	}

	return *this;
}

float256_t abs(float256_t x)
{
	x._ph()->sign = 0;
	return x;
}

_KSN_END



void print_fp256(const ksn::float256_t& x, const char* post = "", const char* pre = "", bool force_sign = false, bool hex = false, FILE* fd = stdout)
{
	if (pre != 0)
		fwrite(pre, sizeof(char), strlen(pre), fd);
	if (force_sign)
	{
		fwrite(x._ph()->sign ? "-" : "+", sizeof(char), 1, fd);
	}
	else if (x._ph()->sign)
		fwrite("-", sizeof(char), 1, fd);

	int exp_v = 0;

	if (hex)
	{
		fprintf(fd, "No");
	}
	else
	{
		uint8_t q[30], r, zero[30] = { 0 };
		memcpy(q, &x, sizeof(q));

		char buffer[128];
		char* pbuf = buffer + 128;
		*--pbuf = '\0';

		if (memcmp(q, zero, sizeof(q)) == 0)
		{
			fprintf(fd, "0");
			exp_v = 1;
		}
		else
		{
			do
			{
				ksn::arr_div<30>(q, &r, q, 10);
				*--pbuf = '0' + r;
				exp_v++;
			} while (memcmp(q, zero, sizeof(q)) != 0);
			if (exp_v != 1)
			{
				--pbuf;
				pbuf[0] = pbuf[1];
				pbuf[1] = '.';
			}
			fprintf(fd, "%s", pbuf);
		}
	}
	fprintf(fd, "e%+hi", x._pe()->exponent + exp_v - 1);
	if (post != 0)
		fwrite(post, sizeof(char), strlen(post), fd);
}



_KSN_BEGIN

struct rational
{
	int64_t numerator;
	uint64_t denominator;

	rational();
	rational(const rational&) noexcept;
	rational(rational&&) noexcept;

	~rational();
};

_KSN_END



int main()
{

	static constexpr double magic = 2.920050977316;
	double x = magic;

	for (int i = 0; i < 100; ++i)
	{
		printf("%lg ", floor(x));
		x = floor(x) * (x - floor(x) + 1);
	}

	getchar();
}
