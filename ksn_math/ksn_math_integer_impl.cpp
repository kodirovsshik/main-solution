// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*

TODO: Review operators >> and << (linteger a, linteger b) in case b.length() != 1

*/

#include <ksn/math_long_integer.hpp>
#include <ksn/debug_utils.hpp>

#include <string.h>
#include <math.h>
#include <stdexcept>

#include <windows.h>
#include <excpt.h>
#include <intrin.h>

#include "ksn_integer_impl_defines.hpp"


_KSN_BEGIN





static void default_deallocator(void *p, size_t)
{
	free(p);
}





size_t linteger::g_s_initial_capacity = 4;
int linteger::g_s_initial_capacity_flags = 0x10101;
bool linteger::g_s_init_with_zero = true;

void* (*linteger::g_s_allocator_function)(size_t) = &malloc;
void (*linteger::g_s_deallocator_function)(void*, size_t) = &default_deallocator;





static void* linteger_internal_allocator_wrapper(size_t n)
{
	MessageBoxA(0, "TODO", "TODO", 0);
	if (n == 0)
	{
		return nullptr;
	}

	void *p = linteger::g_s_allocator_function(n);
	if (p == nullptr)
	{
		_KSN_RAISE(std::bad_alloc());
	}

	return p;
}
static void linteger_internal_deallocator_wrapper(void *p, size_t n)
{
	if (p && n)
	{
		linteger::g_s_deallocator_function(p, n);
	}
}





_KSN_DETAIL_BEGIN

void internal_linteger_add(uint64_t *out, const uint64_t *in1, const uint64_t *in2, size_t so, size_t s1, size_t s2, uint64_t sign1, uint64_t sign2);

void internal_linteger_substract(uint64_t *out, const uint64_t *in1, const uint64_t *in2, size_t so, size_t s1, size_t s2, uint64_t sign1, uint64_t sign2);

void internal_linteger_multiply_unsigned(uint64_t *out, uint64_t temp, const uint64_t *in1, const uint64_t *in2, size_t so, size_t s1, size_t s2);

void internal_linteger_divide_unsigned(uint64_t *out, uint64_t *remainder, const uint64_t *in1, const uint64_t *in2, size_t so, size_t s1, size_t s2, size_t l1, size_t l2);

void linteger_internal_or(uint64_t *p, const uint64_t *p1, const uint64_t *p2, size_t so, size_t s1, size_t s2, uint64_t sign1, uint64_t sign2)
{
	uint64_t *pe = p + so;
	const uint64_t *pe1 = p1 + s1, * pe2 = p2 + s2;

	while (p != pe)
	{
		uint64_t temp;

		if (p1 < pe1)
		{
			temp = *p1;
		}
		else
		{
			temp = sign1;
		}

		if (p2 < pe2)
		{
			temp |= *p2;
		}
		else
		{
			temp |= sign2;
		}

		*p++ = temp;
		p1++; p2++;
	}
}

void linteger_internal_and(uint64_t *p, const uint64_t *p1, const uint64_t *p2, size_t so, size_t s1, size_t s2, uint64_t sign1, uint64_t sign2)
{
	uint64_t *pe = p + so;
	const uint64_t *pe1 = p1 + s1, * pe2 = p2 + s2;

	while (p != pe)
	{
		uint64_t temp;

		if (p1 < pe1)
		{
			temp = *p1;
		}
		else
		{
			temp = sign1;
		}

		if (p2 < pe2)
		{
			temp &= *p2;
		}
		else
		{
			temp &= sign2;
		}

		*p++ = temp;
		p1++; p2++;
	}
}

void linteger_internal_xor(uint64_t *p, const uint64_t *p1, const uint64_t *p2, size_t so, size_t s1, size_t s2, uint64_t sign1, uint64_t sign2)
{
	uint64_t *pe = p + so;
	const uint64_t *pe1 = p1 + s1, * pe2 = p2 + s2;

	while (p != pe)
	{
		uint64_t temp;

		if (p1 < pe1)
		{
			temp = *p1;
		}
		else
		{
			temp = sign1;
		}

		if (p2 < pe2)
		{
			temp ^= *p2;
		}
		else
		{
			temp ^= sign2;
		}

		*p++ = temp;
		p1++; p2++;
	}
}

void linteger_internal_shr(ksn::linteger* _this, int64_t bits, int64_t sign);

void linteger_internal_shl(ksn::linteger* _this, int64_t bits, int64_t sign);

_KSN_DETAIL_END





void linteger::restore_default_allocator()
{
	linteger::g_s_allocator_function = &malloc;
}
void linteger::restore_default_deallocator()
{
	linteger::g_s_deallocator_function = &default_deallocator;
}



linteger::~linteger()
{
	linteger_internal_deallocator_wrapper(this->m_p_data, this->m_capacity * BYTES_PER_UNIT);
}

linteger::linteger()
{
	this->_first_init(linteger::g_s_initial_capacity);

	if (linteger::g_s_init_with_zero)
	{
		memset(this->m_p_data, 0, BYTES_PER_UNIT * this->m_capacity);
	}
}

linteger::linteger(capacity_construct_t capacity)
{
	this->_first_init(capacity.value);

	if (linteger::g_s_init_with_zero)
	{
		memset(this->m_p_data, 0, BYTES_PER_UNIT * capacity.value);
	}
}

linteger::linteger(const linteger& other)
{
	this->_first_init(other.m_capacity);
	memcpy(this->m_p_data, other.m_p_data, BYTES_PER_UNIT * other.m_capacity);
	this->g_capacity_flags = other.g_capacity_flags;
}

linteger::linteger(linteger&& other) noexcept
{
	this->m_p_data = other.m_p_data;
	this->m_capacity = other.m_capacity;
	this->m_length = other.m_length;
	this->g_capacity_flags = other.g_capacity_flags;

	other.m_p_data = nullptr;
	other.m_capacity = 0;
	other.m_length = 0;
}

linteger::linteger(const trivial_arithmetic_type& x, size_t c)
{
	this->_first_init(c);

	if (x.m_type > 2)
	{
		return;
	}

	switch (x.m_type)
	{
	case 0:
		this->construct_from_signed(x.m_value_signed);
		break;

	case 1:
		this->construct_from_unsigned(x.m_value_unsigned);
		break;

	case 2:
		this->construct_from_double(x.m_value_floating);
		break;
	}
}

#pragma warning(push)
#pragma warning(disable : 26495)

linteger::linteger(uninitialized_t) {}

#pragma warning(pop)




linteger& linteger::operator=(const linteger& other)
{
	linteger_internal_deallocator_wrapper(this->m_p_data, this->m_capacity);
	this->m_p_data = (uint64_t*)linteger_internal_allocator_wrapper(other.m_capacity * BYTES_PER_UNIT);
	memcpy(this->m_p_data, other.m_p_data, other.m_capacity * BYTES_PER_UNIT);

	this->m_length = other.m_length;
	this->g_capacity_flags = other.g_capacity_flags;

	return *this;
}

linteger& linteger::operator=(linteger&& other) noexcept
{
	this->m_p_data = other.m_p_data;
	this->m_capacity = other.m_capacity;
	this->m_length = other.m_length;
	this->g_capacity_flags = other.g_capacity_flags;

	other.m_capacity = other.m_length = 0;

	return *this;
}





void linteger::_first_init(size_t cap)
{
	this->m_capacity = cap;

	if (!cap)
	{
		this->m_p_data = 0;
		return;
	}

	this->m_p_data = (uint64_t*)linteger_internal_allocator_wrapper(BYTES_PER_UNIT * this->m_capacity);
}

struct linteger::capacity_construct_t linteger::from_capacity(size_t x)
{
	capacity_construct_t a;
	a.value = x;
	return a;
}


void linteger::construct_from_signed(int64_t x)
{
	construct_if_empty(this);

	this->m_p_data[0] = x;
	memset(this->m_p_data + 1, x < 0 ? -1 : 0, BYTES_PER_UNIT * (this->m_capacity - 1));
	this->m_length = 1;
}

void linteger::construct_from_unsigned(uint64_t x)
{
	construct_if_empty(this);

	this->m_p_data[0] = x;
	memset(this->m_p_data + 1, 0, BYTES_PER_UNIT * (this->m_capacity - 1));
	this->m_length = 1;
}

void linteger::construct_from_double(long double x)
{
	construct_if_empty(this);

	if (x == 0)
	{
		memset(this->m_p_data, 0, this->m_capacity * BYTES_PER_UNIT);
		this->m_length = 1;
		return;
	}

	size_t minimal_capacity = size_t(::ceil(::log2(x < 0 ? -x : x))) + 1;
#pragma warning(push)
#pragma warning(disable : 4800)
	minimal_capacity = minimal_capacity / BITS_PER_UNIT + size_t(bool(minimal_capacity));
#pragma warning(pop)

	this->_extend_reallocate(minimal_capacity);

	size_t length = 0;
	uint64_t *p = this->m_p_data;
	uint64_t negator = 0;

	if (x < 0)
	{
		x = -x;
		negator = -1;
	}


	//2^64 == 18446744073709551616.0
	while (x >= 1)
	{
		*p++ = uint64_t(fmod(x, 18446744073709551616.0)) ^ negator;
		x /= 18446744073709551616.0;
		length++;
	}


	//Fill with sign qword
	memset(this->m_p_data + length, uint8_t(negator), (this->m_capacity - length) * BYTES_PER_UNIT);
	//for (uint64_t *p = this->m_p_data + length; p != this->m_p_data + this->m_capacity; ++p)
	//{
	//	*p = negator;
	//}


	//Here i cut down high dword because on 32-bit platforms it will make cpu
	//not to perform two comparisons instead of one which is needed indeed
	//Theoretically, a compiler have to be able to do this little optimization, but i prefer play safe

	//Why are u looking at me like this? When making a long arithmetic class, any nanosecond IS expensive

	if (uint32_t(negator))
	{
		uint64_t *p = this->m_p_data;

		size_t i = 0;
		while (i < length)
		{
			++*p;
			if (*p)
			{
				break;
			}
			++i;
		}
	}

	this->m_length = length;
}

void linteger::construct_from_integer(const trivial_integer_type& x)
{
	if (x.m_is_signed)
	{
		this->construct_from_signed(x.m_value_signed);
	}
	else
	{
		this->construct_from_unsigned(x.m_value_unsigned);
	}
}

void linteger::construct_from_trivial(const trivial_arithmetic_type& x)
{
	switch (x.m_type)
	{
	case 0:
		return this->construct_from_signed(x.m_value_signed);

	case 1:
		return this->construct_from_unsigned(x.m_value_unsigned);

	case 2:
		return this->construct_from_double(x.m_value_floating);

	//default:
	//	abort();
	}
}





void linteger::shrink(size_t maximal_capacity)
{
	if (maximal_capacity == 0)
	{
		maximal_capacity = 1;
	}

	if (this->m_capacity > maximal_capacity)
	{
		uint64_t *p_new = (uint64_t*)linteger_internal_allocator_wrapper(maximal_capacity * BYTES_PER_UNIT);

		memcpy(p_new, this->m_p_data, BYTES_PER_UNIT * maximal_capacity);

		linteger_internal_deallocator_wrapper(this->m_p_data, BYTES_PER_UNIT * this->m_capacity);

		this->m_p_data = p_new;
		this->m_capacity = maximal_capacity;

		if (this->m_length > maximal_capacity)
		{
			this->m_length = maximal_capacity;
		}
	}
}

void linteger::_shrink(size_t x)
{
	if (x + 1 == this->m_capacity)
	{
		return;
	}

	this->shrink(x);
}

void linteger::extend(size_t minimal_capacity)
{
	if (this->m_capacity < minimal_capacity)
	{
		uint64_t *p_new = (uint64_t*)linteger_internal_allocator_wrapper(BYTES_PER_UNIT * minimal_capacity);

		memcpy(p_new, this->m_p_data, this->m_capacity * BYTES_PER_UNIT);
		memset(
			p_new + this->m_capacity,
			this->sign32(),
			BYTES_PER_UNIT * (minimal_capacity - this->m_capacity)
		);

		linteger_internal_deallocator_wrapper(this->m_p_data, BYTES_PER_UNIT * this->m_capacity);

		this->m_capacity = minimal_capacity;
		this->m_p_data = p_new;
	}
}

void linteger::_extend_no_sign(size_t minimal_capacity)
{
	if (this->m_capacity < minimal_capacity)
	{
		uint64_t *p_new = (uint64_t*)linteger_internal_allocator_wrapper(BYTES_PER_UNIT * minimal_capacity);
		memcpy(p_new, this->m_p_data, this->m_capacity * BYTES_PER_UNIT);
		linteger_internal_deallocator_wrapper(this->m_p_data, BYTES_PER_UNIT * this->m_capacity);

		this->m_capacity = minimal_capacity;
		this->m_p_data = p_new;
	}
}

void linteger::_extend_reallocate(size_t minimal_capacity)
{
	if (this->m_capacity < minimal_capacity)
	{
		linteger_internal_deallocator_wrapper(this->m_p_data, this->m_capacity * BYTES_PER_UNIT);
		this->m_p_data = (uint64_t*)linteger_internal_allocator_wrapper(minimal_capacity);
		this->m_capacity = minimal_capacity;
	}
}

void linteger::reset()
{
	linteger_internal_deallocator_wrapper(this->m_p_data, this->m_capacity * BYTES_PER_UNIT);
	this->m_length = this->m_capacity = 0;
}

void linteger::zero()
{
	if (this->g_flags.autoshrink)
	{
		if (this->m_capacity != 1)
		{
			linteger_internal_deallocator_wrapper(this->m_p_data, this->m_capacity);
			this->m_p_data = (uint64_t*)linteger_internal_allocator_wrapper(BYTES_PER_UNIT);
			this->m_length = this->m_capacity = 1;
		}
	}

	memset(this->m_p_data, 0, this->m_capacity * BYTES_PER_UNIT);
}

void linteger::sign_switch()
{
	this->sign_switch_1s_compliment();
	++*this;
}

void linteger::sign_switch_1s_compliment()
{
	for (uint64_t *p = this->m_p_data; p != this->m_p_data + this->m_capacity; ++p)
	{
		*p ^= -1;
	}
}





linteger operator+(const linteger& a, const linteger& b)
{
	linteger result(a);
	set_result_flags(result, a, b);
	result += b;
	return result;
}
linteger operator-(const linteger& a, const linteger& b)
{
	linteger result(a);
	set_result_flags(result, a, b);
	result -= b;
	return result;
}

linteger operator<<(const linteger& a, const linteger& b)
{
	linteger result(a);

	if (b.m_capacity == 0)
	{
		return result;
	}

	if (b.length() != 1)
	{
		if (b.sign())
		{
			result.sar(INT64_MAX);
		}
		else
		{
			result.sal(INT64_MAX);
		}
	}

	result.sal(*b.m_p_data);

	return a;
}
linteger operator>>(const linteger& a, const linteger& b)
{
	linteger result(a);

	if (b.m_capacity == 0)
	{
		return result;
	}

	if (b.length() != 1)
	{
		if (b.sign())
		{
			result.sal(INT64_MAX);
		}
		else
		{
			result.sar(INT64_MAX);
		}
	}

	result.sar(*b.m_p_data);

	return a;
}

linteger operator|(const linteger& a, const linteger& b)
{
	linteger result(a);
	set_result_flags(result, a, b);
	result |= b;
	return result;
}
linteger operator&(const linteger& a, const linteger& b)
{
	linteger result(a);
	set_result_flags(result, a, b);
	result &= b;
	return result;
}
linteger operator^(const linteger& a, const linteger& b)
{
	linteger result(a);
	set_result_flags(result, a, b);
	result ^= b;
	return result;
}





linteger operator+(const linteger& a, const trivial_integer_type& b)
{
	linteger result(a);
	result += b;
	return result;
}
linteger operator-(const linteger& a, const trivial_integer_type& b)
{
	linteger result(a);
	result -= b;
	return result;
}

linteger operator<<(const linteger& a, int64_t b)
{
	linteger result(a);
	result.sal(b);
	return result;
}
linteger operator>>(const linteger& a, int64_t b)
{
	linteger result(a);
	result.sar(b);
	return result;
}

linteger operator|(const linteger& a, const trivial_integer_type& b)
{
	linteger result(a);
	result |= b;
	return result;
}
linteger operator&(const linteger& a, const trivial_integer_type& b)
{
	linteger result(a);
	result &= b;
	return result;
}
linteger operator^(const linteger& a, const trivial_integer_type& b)
{
	linteger result(a);
	result ^= b;
	return result;
}

bool operator||(const linteger& a, const trivial_arithmetic_type& b)
{
	if ((b.m_type & ~1) == 0)
	{
		if (b.m_value_unsigned)
		{
			return true;
		}
	}
	else if (b.m_value_floating)
	{
		return true;
	}

	return bool(a);
}
bool operator&&(const linteger& a, const trivial_arithmetic_type& b)
{
	if ((b.m_type & ~1) == 0)
	{
		if (!b.m_value_unsigned)
		{
			return false;
		}
	}
	else if (!b.m_value_floating)
	{
		return false;
	}

	return bool(a);
}





linteger operator+(const trivial_integer_type& a, const linteger& b)
{
	linteger result(a, b.m_capacity);
	result.g_capacity_flags = b.g_capacity_flags;
	result += b;
	return result;
}
linteger operator-(const trivial_integer_type& a, const linteger& b)
{
	linteger result(a, b.m_capacity);
	result.g_capacity_flags = b.g_capacity_flags;
	result -= b;
	return result;
}
linteger operator*(const trivial_integer_type& a, const linteger& b);
linteger operator/(const trivial_integer_type& a, const linteger& b);
linteger operator%(const trivial_integer_type& a, const linteger& b);

linteger operator<<(const trivial_integer_type& a, const linteger& b)
{
	linteger x(a);
	x <<= b;
	return x;
}
linteger operator>>(const trivial_integer_type& a, const linteger& b)
{
	linteger x(a);
	x >>= b;
	return x;
}

linteger operator|(const trivial_integer_type& a, const linteger& b)
{
	return b | a;
}
linteger operator&(const trivial_integer_type& a, const linteger& b)
{
	return b & a;
}
linteger operator^(const trivial_integer_type& a, const linteger& b)
{
	return b ^ a;
}

bool operator||(const trivial_arithmetic_type& a, const linteger& b)
{
	return b || a;
}
bool operator&&(const trivial_arithmetic_type& a, const linteger& b)
{
	return b && a;
}





//Shiht arithmetical right
void linteger::sar(int64_t bits)
{
	construct_if_empty(this);
	return detail::linteger_internal_shr(this, bits, this->sign64());
}
void linteger::sar(const linteger& x)
{
	construct_if_empty(this);

	this->sar(int64_t(x));
}

//Shiht logical right
void linteger::shr(int64_t bits)
{
	construct_if_empty(this);
	return detail::linteger_internal_shr(this, bits, 0);
}
void linteger::shr(const linteger& x)
{
	construct_if_empty(this);

	this->shr(int64_t(x));
}

//Shiht arithmetical left
void linteger::sal(int64_t bits)
{
	construct_if_empty(this);
	return detail::linteger_internal_shl(this, bits, this->sign64());
}
void linteger::sal(const linteger& x)
{
	construct_if_empty(this);

	this->sal(int64_t(x));
}

//Shiht logical left
void linteger::shl(int64_t bits)
{
	construct_if_empty(this);
	return detail::linteger_internal_shl(this, bits, 0);
}
void linteger::shl(const linteger& x)
{
	construct_if_empty(this);

	this->shl(int64_t(x));
}





size_t linteger::length() const
{
	if (this->m_capacity != 0 && this->m_length == 0)
	{
		uint64_t sign = this->sign64();

		uint64_t *p = this->m_p_data + this->m_capacity - 1;

		while (p > this->m_p_data)
		{
			if (*p != sign)
			{
				break;
			}
			--p;
		}

		return this->m_length = p - this->m_p_data + 1;
	}

	return this->m_length;
}

bool linteger::sign() const
{
	if (this->m_capacity == 0)
	{
		return false;
	}

	return int64_t(this->m_p_data[this->m_capacity - 1]) < 0;
}

uint32_t linteger::sign32() const
{
	if (this->m_capacity == 0)
	{
		return 0;
	}
	//return (uint32_t)-(int32_t)(((int32_t*)(this->m_p_data + this->m_capacity - 1))[1] < 0);
	return (uint32_t)( ((int32_t*)(this->m_p_data))[2 * this->m_capacity - 1] >> 31 );
}

uint64_t linteger::sign64() const
{
	if (this->m_capacity == 0)
	{
		return 0;
	}
	//return (uint64_t)-(int64_t)( ((int32_t*)(this->m_p_data + this->m_capacity - 1))[1] < 0);
	return (uint64_t)((int64_t)this->m_p_data[this->m_capacity - 1] >> 63);
}




linteger& linteger::operator+=(const linteger& x)
{
	construct_if_empty(this);

	if (this->g_flags.autoextend_self || this->g_flags.autoshrink)
	{
		size_t len1 = this->length();
		size_t len2 = x.length();

		//'o' for "output"
		size_t len_o = (len1 > len2 ? len1 : len2) + 1;

		if (this->g_flags.autoextend_self)
		{
			this->extend(len_o);
		}

		if (this->g_flags.autoshrink)
		{
			this->shrink(len_o);
		}
	}

	detail::internal_linteger_add(this->m_p_data, this->m_p_data, x.m_p_data, this->m_capacity, this->m_capacity, x.m_capacity, this->sign64(), x.sign64());

	if (this->m_length != this->m_capacity && this->m_p_data[this->m_length] != (uint64_t)-(int64_t)(int64_t(this->m_p_data[m_capacity - 1]) < 0)) //data[len] != sign
	{
		++this->m_length;
	}

	return *this;
}
linteger& linteger::operator-=(const linteger& x)
{
	construct_if_empty(this);

	if (this->g_flags.autoextend_self || this->g_flags.autoshrink)
	{
		size_t len1 = this->length();
		size_t len2 = x.length();

		//'o' for "output"
		size_t len_o = (len1 > len2 ? len1 : len2) + 1;

		if (this->g_flags.autoextend_self)
		{
			this->extend(len_o);
		}

		if (this->g_flags.autoshrink)
		{
			this->shrink(len_o);
		}
	}

	detail::internal_linteger_substract(
		this->m_p_data, this->m_p_data, x.m_p_data,
		this->m_capacity, this->m_capacity, x.m_capacity,
		this->sign64(), x.sign64()
	);

	if (this->m_length != this->m_capacity && this->m_p_data[this->m_length] != (uint64_t)-(int64_t)(int64_t(this->m_p_data[m_capacity - 1]) < 0)) //data[len] != sign
	{
		++this->m_length;
	}

	return *this;
}
linteger& linteger::operator*=(const linteger& x)
{
	construct_if_empty(this);

	if (this->g_flags.autoextend_self || this->g_flags.autoshrink)
	{
		size_t len1 = this->length();
		size_t len2 = x.length();

		if (this->g_flags.autoextend_self)
		{
			this->extend(len1 + len2 + 1);
		}

		if (this->g_flags.autoshrink)
		{
			this->shrink(len1 + len2 + 1);
		}
	}

	throw 0;

	return *this;
}

linteger& linteger::operator<<=(const linteger& x)
{
	this->shl(x);
	return *this;
}
linteger& linteger::operator>>=(const linteger& x)
{
	this->sar(x);
	return *this;
}

linteger& linteger::operator|=(const linteger& x)
{
	if (this->g_flags.autoextend_self)
	{
		this->extend(x);
	}

	detail::linteger_internal_or(
		this->m_p_data, this->m_p_data, x.m_p_data,
		this->m_capacity, this->m_capacity, x.m_capacity,
		this->sign64(), x.sign64()
	);

	if (this->g_flags.autoshrink)
	{
		this->shrink(this->length());
	}

	return *this;
}
linteger& linteger::operator&=(const linteger& x)
{
	if (this->g_flags.autoextend_self)
	{
		this->extend(x);
	}

	detail::linteger_internal_and(
		this->m_p_data, this->m_p_data, x.m_p_data,
		this->m_capacity, this->m_capacity, x.m_capacity,
		this->sign64(), x.sign64()
	);

	if (this->g_flags.autoshrink)
	{
		this->shrink(this->length());
	}

	return *this;
}
linteger& linteger::operator^=(const linteger& x)
{
	if (this->g_flags.autoextend_self)
	{
		this->extend(x);
	}

	detail::linteger_internal_xor(
		this->m_p_data, this->m_p_data, x.m_p_data,
		this->m_capacity, this->m_capacity, x.m_capacity,
		this->sign64(), x.sign64()
	);

	if (this->g_flags.autoshrink)
	{
		this->shrink(this->length());
	}

	return *this;
}





linteger& linteger::operator+=(const trivial_integer_type& other)
{
	construct_if_empty(this);

	if (this->g_flags.autoextend_self || this->g_flags.autoshrink)
	{
		size_t len = this->length();

		len = (len > 1 ? len : 1) + 1;

		if (this->g_flags.autoextend_self)
		{
			this->extend(len);
		}

		if (this->g_flags.autoshrink)
		{
			this->shrink(len);
		}
	}

	detail::internal_linteger_add(
		this->m_p_data, this->m_p_data, &other.m_value_unsigned, 
		this->m_capacity, this->m_capacity, 1, 
		this->sign64(), other.m_is_signed ? (other.m_value_signed < 0 ? -1 : 0) : 0
	);

	if (this->m_length != this->m_capacity && this->m_p_data[this->m_length] != (uint64_t)-(int64_t)(int64_t(this->m_p_data[m_capacity - 1]) < 0)) //data[len] != sign
	{
		++this->m_length;
	}

	return *this;
}
linteger& linteger::operator-=(const trivial_integer_type& other)
{
	construct_if_empty(this);

	if (this->g_flags.autoextend_self || this->g_flags.autoshrink)
	{
		size_t len = this->length();

		len = (len > 1 ? len : 1) + 1;

		if (this->g_flags.autoextend_self)
		{
			this->extend(len);
		}

		if (this->g_flags.autoshrink)
		{
			this->shrink(len);
		}
	}

	detail::internal_linteger_substract(
		this->m_p_data, this->m_p_data, &other.m_value_unsigned,
		this->m_capacity, this->m_capacity, 1,
		this->sign64(), other.m_is_signed ? (other.m_value_signed < 0 ? -1 : 0) : 0
	);

	if (this->m_length != this->m_capacity && this->m_p_data[this->m_length] != (uint64_t)-(int64_t)(int64_t(this->m_p_data[m_capacity - 1]) < 0)) //data[len] != sign
	{
		++this->m_length;
	}

	return *this;
}

linteger& linteger::operator<<=(int64_t x)
{
	this->sal(x);
	return *this;
}
linteger& linteger::operator>>=(int64_t x)
{
	this->sar(x);
	return *this;
}

linteger& linteger::operator|=(const trivial_integer_type& other)
{
	construct_if_empty(this);

	detail::linteger_internal_or(
		this->m_p_data, this->m_p_data, &other.m_value_unsigned,
		this->m_capacity, this->m_capacity, 1,
		this->sign64(), other.m_is_signed ? (other.m_value_signed < 0 ? -1 : 0) : 0
	);

	return *this;
}
linteger& linteger::operator&=(const trivial_integer_type& other)
{
	construct_if_empty(this);

	detail::linteger_internal_and(
		this->m_p_data, this->m_p_data, &other.m_value_unsigned,
		this->m_capacity, this->m_capacity, 1,
		this->sign64(), other.m_is_signed ? (other.m_value_signed < 0 ? -1 : 0) : 0
	);

	return *this;
}
linteger& linteger::operator^=(const trivial_integer_type& other)
{
	construct_if_empty(this);

	detail::linteger_internal_xor(
		this->m_p_data, this->m_p_data, &other.m_value_unsigned,
		this->m_capacity, this->m_capacity, 1,
		this->sign64(), other.m_is_signed ? (other.m_value_signed < 0 ? -1 : 0) : 0
	);

	return *this;
}



bool operator||(const linteger& a, const linteger& b)
{
	if (&a == &b)
	{
		return bool(a);
	}

	uint64_t *pa = a.m_p_data, * pb = b.m_p_data;
	uint64_t *pae = pa + a.m_capacity - 1, * pbe = pb + b.m_capacity - 1;

	while (pa <= pae && pb <= pbe)
	{
		if (*pa || *pae || *pb || *pbe)
		{
			return true;
		}

		pa++; pb++;
		pae--; pbe--;
	}

	while (pa < pae)
	{
		if (*pa)
		{
			return true;
		}
		++pa;
		--pae;
	}

	while (pb < pbe)
	{
		if (*pb)
		{
			return true;
		}

		++pb;
		--pbe;
	}

	return false;
}
bool operator&&(const linteger& a, const linteger& b)
{
	if ((a.m_capacity & b.m_capacity) == 0)
	{
		return false;
	}

	if (&a == &b)
	{
		return bool(a);
	}

	if (bool(a) == false)
	{
		return false;
	}

	return bool(b);
}





linteger linteger::operator++(int)
{
	linteger copy(*this);
	++*this;
	return copy;
}
linteger linteger::operator--(int)
{
	linteger copy(*this);
	--*this;
	return copy;
}
linteger& linteger::operator++()
{
	construct_if_empty(this);

	uint64_t *p = this->m_p_data, * pe = p + this->m_capacity - 1;

	while (p < pe)
	{
		++*p;
		p++;
		if (p[-1] != 0)
		{
			return *this;
		}
	};

	if (p == pe)
	{
		if (*p == 0x7FFFFFFFFFFFFFFF && this->g_flags.autoextend_self)
		{
			this->extend(this->m_capacity + 1);
			p = this->m_p_data + this->m_capacity - 2;
		}

		++*p;
	}

	return *this;
}
linteger& linteger::operator--()
{
	construct_if_empty(this);

	uint64_t *p = this->m_p_data, * pe = p + this->m_capacity - 1;

	while (p < pe)
	{
		--*p;
		p++;
		if (p[-1] != uint64_t(-1))
		{
			return *this;
		}
	};

	if (p == pe)
	{
		if (*p == 0x8000000000000000 && this->g_flags.autoextend_self)
		{
			this->extend(this->m_capacity + 1);
			p = this->m_p_data + this->m_capacity - 2;
		}

		--*p;
	}

	return *this;
}





#pragma warning(push)
#pragma warning(disable : 4700 4244 6001)
//4700 and 6001 for "Uninitialized variable used"
//4244 for "Implicit cast"

linteger::operator int8_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}
linteger::operator int16_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}
linteger::operator int32_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}
linteger::operator int64_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}

linteger::operator uint8_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}
linteger::operator uint16_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}
linteger::operator uint32_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}
linteger::operator uint64_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}

linteger::operator float() const
{
	return (float)(long double)(*this);
}
linteger::operator double() const
{
	return (double)(long double)(*this);
}
linteger::operator long double() const
{
	if (this->m_capacity == 0)
	{
		long double x;
		return x; //-V614
	}

	long double result = 0;

	bool negative = this->sign();
	long double multiplier = negative ? -1 : 1;
	int64_t negator = negative ? -1 : 0;

	int carry = -(int)negator;

	for (uint64_t *p = this->m_p_data; p != this->m_p_data + this->m_capacity; ++p)
	{
		//int64_t x = *p;
		//x ^= negator;
		//x += carry;
		//result += x * multiplier;
		result += ((*p ^ negator) + carry) * multiplier;
		carry = carry && !*p;
		multiplier *= 18446744073709551616.0L;
	}

	return result;
}

linteger::operator char() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}
linteger::operator wchar_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}
linteger::operator char16_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}
linteger::operator char32_t() const
{
	if (this->m_capacity == 0)
	{
		int64_t x;
		return x; //-V614
	}
	return *this->m_p_data;
}

linteger::operator bool() const
{
	if (this->m_capacity == 0)
	{
		bool b;
		return b; //-V614
	}

	uint64_t *p1 = this->m_p_data + this->m_capacity - 1;
	uint64_t *p2 = this->m_p_data;

	while (p1 >= p2)
	{
		if (*p1 || *p2)
		{
			return true;
		}

		--p1;
		++p2;
	}

	return false;
}

#pragma warning(pop)





_KSN_DETAIL_BEGIN

extern const signed char *_log2_lookup_table8;

_KSN_DETAIL_END

//Square root
linteger sqrt(const linteger& x);

//Cube root
linteger qbrt(const linteger& x);

//nth degree root
linteger root(const linteger& x, uint32_t degree);


//Raises base to the power of exponent
linteger pow(const linteger& base, uint32_t exponent);


//Base 2 logarithm
size_t log2(const linteger& x)
{
	if (x.m_capacity == 0)
	{
		return -1;
	}

	if (int64_t(x.m_p_data[x.m_capacity - 1]) < 0)
	{
		_KSN_RAISE(std::domain_error("Real-valued logarithm is only defined for x > 0"));
	}

	uint64_t *pb = x.m_p_data, *p = pb + x.m_capacity - 1;
	//Find highest storage unit != 0
	while (p > pb && *p == 0)
	{
		--p;
	}

	if (*p == 0)
	{
		throw std::domain_error("Real-valued logarithm is only defined for x > 0");
		abort();
	}

	size_t result = (p - x.m_p_data) * 64;

	//Temporaries
	uint64_t t1, t2;

	//log2(x << n) = log2(x * 2^n) = log2(x) + log2(2^n) = log2(x) + n

	//if high dword !=0, process it
	if ((t1 = (*p >> 32))) //-V112
	{
		//if its higher word != 0, process it
		if ((t2 = (t1 >> 16)))
		{
			//if selected word contains something in higher byte, process this byte, otherwise process lower byte
			result += (t1 = (t2 >> 8)) ? (32 + 16 + 8 + detail::_log2_lookup_table8[t1]) : (32 + 16 + detail::_log2_lookup_table8[t2]); //-V112
		}
		else //process lower word of higher dword
		{
			result += (t2 = (t1 >> 8)) ? (32 + 8 + detail::_log2_lookup_table8[t2]) : (32 + detail::_log2_lookup_table8[t1]); //-V112
		}
	}
	else
	{
		if ((t2 = (*p >> 16)))
		{
			result += (t1 = (t2 >> 8)) ? (16 + 8 + detail::_log2_lookup_table8[t1]) : (16 + detail::_log2_lookup_table8[t2]);
		}
		else
		{
			result += (t1 = (*p >> 8)) ? (8 + detail::_log2_lookup_table8[t1]) : detail::_log2_lookup_table8[*p];
		}
	}

	return result;
}

//Base 10 logarithm
size_t log10(const linteger& x);

//Base 2^n logarithm
size_t log2n(const linteger& x, size_t exponent);

//Base n logarithm
size_t logn(const linteger& x, size_t base);


//Base 2 fp logarithm
long double log2f(const linteger& x);

//Base 10 fp logarithm
long double log10f(const linteger& x);

//Base e fp logarithm
long double ln(const linteger& x);

//Base n fp logarithm
long double lognf(const linteger& x, long double base);





#define test_assert(true_expression) if (!(true_expression)) { return false; } ((void)0)
#define _track_start() collecting_allocator_tracker::track_start()
#define _track_stop() test_assert(collecting_allocator_tracker::track_stop() == 0)

static bool __self_test2()
{
	struct _backuper
	{

		static void* temp_allocator(size_t x)
		{
			collecting_allocator<uint8_t> alloc;

			return (void*)std::allocator_traits<decltype(alloc)>::allocate(alloc, x);
		};


		static void temp_deallocator(void *p, size_t x)
		{
			collecting_allocator<uint8_t> alloc;

			std::allocator_traits<decltype(alloc)>::deallocate(alloc, (uint8_t*)p, x);
		};


		void* (*p_allocator)(size_t x);
		void (*p_deallocator)(void*, size_t x);
		int p_capacity_flags;
		bool p_zero_construct;
		size_t p_capacity;


		_backuper()
		{
			this->p_allocator = linteger::g_s_allocator_function;
			this->p_deallocator = linteger::g_s_deallocator_function;
			this->p_capacity_flags = linteger::g_s_initial_capacity_flags;
			this->p_zero_construct = linteger::g_s_init_with_zero;
			this->p_capacity = linteger::g_s_initial_capacity;

			linteger::g_s_allocator_function = temp_allocator;
			linteger::g_s_deallocator_function = temp_deallocator;
		}


		~_backuper()
		{
			linteger::g_s_allocator_function = this->p_allocator;
			linteger::g_s_deallocator_function = this->p_deallocator;
			linteger::g_s_initial_capacity_flags = this->p_capacity_flags;
			linteger::g_s_init_with_zero = this->p_zero_construct;
			linteger::g_s_initial_capacity = this->p_capacity;
		}

	} __backuper;



	//Test 1: memory leak when exception is thrown during extension process

	_track_start();

	bool caught = false;
	_KSN_TRY

		linteger x(linteger::from_capacity(4)); //-V112
		x.extend(SIZE_MAX);

	_KSN_CATCH_UNNAMED(std::bad_alloc&)

		caught = true;

	_KSN_CATCH_ALL

		return false;

	_KSN_CATCH_END

	test_assert(caught);

	_track_stop();



	_track_start();

	caught = false;
	try
	{
		linteger x(linteger::from_capacity(1));
		for (size_t i = 2; i; i *= 2)
		{
			x.extend(i);
		}
	}
	catch (std::bad_alloc&)
	{
		caught = true;
	}
	catch (...)
	{
		return false;
	}

	test_assert(caught);

	_track_stop();



	//Test 2: constructors' logic
	linteger::g_s_initial_capacity_flags = 0x010101;
	linteger::g_s_init_with_zero = true;
	linteger::g_s_initial_capacity = 1;

	_track_start();
	{

		linteger a;

		test_assert(a.m_capacity == 1);
		test_assert(a.m_p_data != 0);
		test_assert(a.m_p_data[0] == 0);
		test_assert(a.m_length == 0);
		test_assert(a.length() == 1);
		test_assert(a.m_length == 1);
	}
	_track_stop();



	_track_start();
	{
		linteger::g_s_initial_capacity = 0;
		linteger a;

		test_assert(a.m_capacity == 0);
		test_assert(a.m_p_data == 0);
		test_assert(a.m_length == 0);
		test_assert(a.length() == 0);
		test_assert(a.m_length == 0); //-V649
	}
	_track_stop();



	_track_start();
	{
		linteger::g_s_initial_capacity = 4; //-V112
		linteger a(linteger::from_capacity(0));

		test_assert(a.m_capacity == 0);
		test_assert(a.m_p_data == 0);
		test_assert(a.m_length == 0);
		test_assert(a.length() == 0);
		test_assert(a.m_length == 0); //-V649
	}
	_track_stop();



	//Test n: log2
	_track_start();
	{
		ksn::linteger x(1, 16);
		x.g_capacity_flags = 0;
		for (size_t expected = 0; expected < 1023;) //So many just in case :p
		{
			test_assert(log2(x) == expected);

			expected++;
			x <<= 1;
		}
	}
	_track_stop();

	_track_start();
	{
		bool caught = false;
		try
		{
			ksn::linteger x;
			log2(x);
		}
		catch (std::domain_error&)
		{
			caught = true;
		}
		catch (...)
		{
			return false;
		}

		test_assert(caught);
	}
	_track_stop();

	_track_start();
	{
		bool caught = false;
		try
		{
			ksn::linteger x(-1);
			log2(x);
		}
		catch (std::domain_error&)
		{
			caught = true;
		}
		catch (...)
		{
			return false;
		}

		test_assert(caught);
	}
	_track_stop();



	return true;
}

static bool __self_test1()
{
	try
	{
		_track_start();
		bool result = __self_test2();
		_track_stop();

		return result;
	}
	catch (...)
	{
		return false;
	}
}

bool linteger::__self_test()
{
	bool result;
	__try
	{
		result = __self_test1();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}

	test_assert(collecting_allocator_tracker::track_layers.size() == 0);

	return result;
}

#undef test_assert
#undef _track_start
#undef _track_stop



_KSN_END
