// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <ksn/math_long_integer.hpp>
#include "ksn_integer_impl_defines.hpp"

#include <string.h>

#include <intrin.h>

#include <ksn/debug_utils.hpp>



_KSN_BEGIN

_KSN_DETAIL_BEGIN


//Just to silence runtime error checking messages from Visual Studio
#if _KSN_64

void internal_linteger_add(uint64_t* out, const uint64_t* in1, const uint64_t* in2, size_t so, size_t s1, size_t s2, uint64_t sign1, uint64_t sign2)
{
	bool carry = false;

	for (size_t i = 0; i < so; ++i)
	{
		uint64_t operand1 = (i >= s1 ? sign1 : *in1);
		uint64_t operand2 = (i >= s2 ? sign2 : *in2);

		carry = _adc64(carry, operand1, operand2, out++);

		++in1, ++in2;
	}
}

void internal_linteger_substract(uint64_t* out, const uint64_t* in1, const uint64_t* in2, size_t so, size_t s1, size_t s2, uint64_t sign1, uint64_t sign2)
{
	bool carry = false;

	for (size_t i = 0; i < so; ++i)
	{
		uint64_t operand1 = (i >= s1 ? sign1 : *in1);
		uint64_t operand2 = (i >= s2 ? sign2 : *in2);

		carry = _sbb64(carry, operand1, operand2, out++);

		++in1, ++in2;
	}
}

#endif

void internal_linteger_multiply_unsigned(uint64_t* out, uint64_t temp, const uint64_t* in1, const uint64_t* in2, size_t so, size_t s1, size_t s2)
{
	//Well...
}

void internal_linteger_divide_unsigned(uint64_t* out, uint64_t* remainder, const uint64_t* in1, const uint64_t* in2, size_t so, size_t s1, size_t s2, size_t l1, size_t l2)
{
	// :c
}



void linteger_internal_shl(ksn::linteger*, int64_t, int64_t);
void linteger_internal_shr(ksn::linteger* _this, int64_t bits, int64_t sign)
{
	if (bits == INT64_MIN)
	{
		bits++;
	}

	if (bits < 0)
	{
		return linteger_internal_shl(_this, -bits, sign);
	}

	if (bits == 0)
	{
		return;
	}

	//_this->length();

	size_t move_by_units = size_t(bits / BITS_PER_UNIT);
	size_t move_by_bits = bits % BITS_PER_UNIT;

	if (move_by_units > _this->m_capacity)
	{
		memset(_this->m_p_data, 0, _this->m_capacity * BYTES_PER_UNIT);

		_this->m_length = 1;
	}
	else
	{
		if (move_by_units)
		{
			if (_this->m_length)
			{
				if (move_by_units >= _this->m_length)
				{
					memset(_this->m_p_data, int(sign), _this->m_length * BYTES_PER_UNIT);
				}
				else
				{
					memcpy(_this->m_p_data, _this->m_p_data + move_by_units, (_this->m_length - move_by_units));
					memset(_this->m_p_data + _this->m_length - move_by_units, int(sign), move_by_units * BYTES_PER_UNIT);
				}
			}
			else
			{
				if (move_by_units >= _this->m_capacity)
				{
					memset(_this->m_p_data, int(sign), _this->m_capacity * BYTES_PER_UNIT);
				}
				else
				{
					memcpy(_this->m_p_data, _this->m_p_data + move_by_units, (_this->m_capacity - move_by_units));
					memset(_this->m_p_data + _this->m_capacity - move_by_units, int(sign), move_by_units * BYTES_PER_UNIT);
				}
			}
		}

		if (bits)
		{
			uint64_t* p = _this->m_p_data + _this->m_length;

			size_t back_bits = BITS_PER_UNIT - move_by_bits;
			uint64_t carry = sign << back_bits;

			while (--p >= _this->m_p_data)
			{
				uint64_t temp = *p;
				*p = (*p >> move_by_bits) | carry;
				carry = temp << back_bits;
			}
		}

		_this->m_length -= move_by_units;
	}

	//TODO: shrink, length calculations
}

void linteger_internal_shl1(ksn::linteger* _this, int64_t bits, int64_t sign)
{
	if (bits == INT64_MIN)
	{
		bits++;
	}

	if (bits < 0)
	{
		return linteger_internal_shr(_this, -bits, sign);
	}

	if (bits == 0)
	{
		return;
	}

	_this->length();

	size_t new_length; //Not actual length but the one that is guarantueed to be able to store the new number (except when new length has to be > SIZE_MAX)
	size_t new_units;
	size_t move_by_units = size_t(bits / BITS_PER_UNIT);
	size_t move_by_bits = bits % BITS_PER_UNIT;

	if (_this->g_flags.autoextend_self || _this->g_flags.autoshrink)
	{
		new_units = move_by_units + 1;

		//If a size_t variable can't store new_units
		if (SIZE_MAX < new_units)
		{
			new_length = SIZE_MAX;
		}
		//If a size_t variable can't store new_units + _this->m_length
		else if (SIZE_MAX - new_units < _this->m_length)
		{
			new_length = SIZE_MAX;
		}
		else //if it can
		{
			new_length = size_t(new_units + _this->m_length);
		}
	}

	if (_this->g_flags.autoextend_self)
	{
		_this->_extend_no_sign(new_length);
	}




	if (move_by_units > _this->m_capacity)
	{
		memset(_this->m_p_data, 0, _this->m_capacity * BYTES_PER_UNIT);

		_this->m_length = 1;
	}
	else
	{
		if (move_by_units)
		{
			memcpy(_this->m_p_data + move_by_units, _this->m_p_data, (_this->m_capacity - move_by_units) * BYTES_PER_UNIT);
			memset(_this->m_p_data, 0, move_by_units * BYTES_PER_UNIT);
		}

		if (move_by_bits)
		{
			uint64_t* p = _this->m_p_data + move_by_units, * pe = p + _this->m_length;
			uint64_t* pe2 = _this->m_p_data + _this->m_capacity - 1;
			if (pe > pe2)
			{
				pe = pe2;
			}

			if (_this->m_length != _this->m_capacity)
			{
				uint64_t carry = 0;
				uint64_t lowest_sign_unit_was = _this->m_p_data[_this->m_length];

				while (p <= pe)
				{
					uint64_t temp = *p >> (64 - move_by_bits);
					*p = (*p << move_by_bits) | carry;
					carry = temp;
					++p;
				}

				if (lowest_sign_unit_was != _this->m_p_data[_this->m_length])
				{
					_this->m_length++;
				}
			}
			else
			{
				uint64_t carry = 0;

				while (p <= pe)
				{
					uint64_t temp = *p >> (64 - move_by_bits);
					*p = (*p << move_by_bits) | carry;
					carry = temp;
					++p;
				}
			}
		}

		_this->m_length += move_by_units;
	}



	if (_this->g_flags.autoshrink)
	{
		_this->shrink(new_length);
	}
}
void linteger_internal_shl(ksn::linteger* _this, int64_t bits, int64_t sign)
{
	if (bits == INT64_MIN)
	{
		bits++;
	}

	if (bits < 0)
	{
		return linteger_internal_shr(_this, -bits, sign);
	}

	if (bits == 0)
	{
		return;
	}

	_this->length();

	size_t new_length; //Not actual length but the one that is guarantueed to be able to store the new number (except when new length has to be > SIZE_MAX)
	size_t new_units;
	size_t move_by_units = size_t(bits / BITS_PER_UNIT);
	size_t move_by_bits = bits % BITS_PER_UNIT;

	if (_this->g_flags.autoextend_self || _this->g_flags.autoshrink)
	{
		new_units = move_by_units + 1;

		//If a size_t variable can't store new_units
		if (SIZE_MAX < new_units)
		{
			new_length = SIZE_MAX;
		}
		//If a size_t variable can't store new_units + _this->m_length
		else if (SIZE_MAX - new_units < _this->m_length)
		{
			new_length = SIZE_MAX;
		}
		else //if it can
		{
			new_length = size_t(new_units + _this->m_length);
		}
	}

	if (_this->g_flags.autoextend_self)
	{
		_this->_extend_no_sign(new_length);
	}


	//TODO: clear with zeros low part
	copy_debugger::_break();

	if (move_by_units > _this->m_capacity)
	{
		memset(_this->m_p_data, 0, _this->m_capacity * BYTES_PER_UNIT);

		_this->m_length = 1;
	}
	else
	{
		if (move_by_units && move_by_bits)
		{
			uint64_t* p = _this->m_p_data + move_by_units, * pe = p + _this->m_length, *psrc = _this->m_p_data;
			uint64_t* pe2 = _this->m_p_data + _this->m_capacity - 1;
			if (pe > pe2)
			{
				pe = pe2;
			}

			if (_this->m_length < _this->m_capacity)
			{
				uint64_t carry = 0;
				uint64_t lowest_sign_unit_was = _this->m_p_data[_this->m_length];

				while (p <= pe)
				{
					uint64_t temp = *psrc >> (64 - move_by_bits);
					*p = (*psrc << move_by_bits) | carry;
					carry = temp;
					++p;
					++psrc;
				}

				if (lowest_sign_unit_was != _this->m_p_data[_this->m_length])
				{
					_this->m_length++;
				}
			}
			else
			{
				uint64_t carry = 0;

				while (p <= pe)
				{
					uint64_t temp = *p >> (64 - move_by_bits);
					*p = (*p << move_by_bits) | carry;
					carry = temp;
					++p;
				}
			}
		}
		else if (move_by_units)
		{
			memcpy(_this->m_p_data + move_by_units, _this->m_p_data, (_this->m_capacity - move_by_units) * BYTES_PER_UNIT);
			memset(_this->m_p_data, 0, move_by_units * BYTES_PER_UNIT);
		}
		else if (move_by_bits)
		{
			uint64_t* p = _this->m_p_data + move_by_units, * pe = p + _this->m_length;
			uint64_t* pe2 = _this->m_p_data + _this->m_capacity - 1;
			if (pe > pe2)
			{
				pe = pe2;
			}

			if (_this->m_length != _this->m_capacity)
			{
				uint64_t carry = 0;
				uint64_t lowest_sign_unit_was = _this->m_p_data[_this->m_length];

				while (p <= pe)
				{
					uint64_t temp = *p >> (64 - move_by_bits);
					*p = (*p << move_by_bits) | carry;
					carry = temp;
					++p;
				}

				if (lowest_sign_unit_was != _this->m_p_data[_this->m_length])
				{
					_this->m_length++;
				}
			}
			else
			{
				uint64_t carry = 0;

				while (p <= pe)
				{
					uint64_t temp = *p >> (64 - move_by_bits);
					*p = (*p << move_by_bits) | carry;
					carry = temp;
					++p;
				}
			}
		}

		_this->m_length += move_by_units;
	}



	if (_this->g_flags.autoshrink)
	{
		_this->shrink(new_length);
	}
}


_KSN_DETAIL_END

_KSN_END
