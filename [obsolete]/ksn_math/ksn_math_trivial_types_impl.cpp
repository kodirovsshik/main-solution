// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <ksn/math.hpp>



_KSN_BEGIN


trivial_signed_type::trivial_signed_type(int8_t x) : m_value(x) {}
trivial_signed_type::trivial_signed_type(int16_t x) : m_value(x) {}
trivial_signed_type::trivial_signed_type(int32_t x) : m_value(x) {}
trivial_signed_type::trivial_signed_type(int64_t x) : m_value(x) {}



trivial_unsigned_type::trivial_unsigned_type(uint8_t x) : m_value(x) {}
trivial_unsigned_type::trivial_unsigned_type(uint16_t x) : m_value(x) {}
trivial_unsigned_type::trivial_unsigned_type(uint32_t x) : m_value(x) {}
trivial_unsigned_type::trivial_unsigned_type(uint64_t x) : m_value(x) {}



trivial_floating_type::trivial_floating_type(float x) : m_value(x) {}
trivial_floating_type::trivial_floating_type(double x) : m_value(x) {}
trivial_floating_type::trivial_floating_type(long double x) : m_value(x) {}



trivial_integer_type::trivial_integer_type(int8_t x) : m_value_signed(x), m_is_signed(true) {}
trivial_integer_type::trivial_integer_type(int16_t x) : m_value_signed(x), m_is_signed(true) {}
trivial_integer_type::trivial_integer_type(int32_t x) : m_value_signed(x), m_is_signed(true) {}
trivial_integer_type::trivial_integer_type(int64_t x) : m_value_signed(x), m_is_signed(true) {}

trivial_integer_type::trivial_integer_type(uint8_t x) : m_value_unsigned(x), m_is_signed(false) {}
trivial_integer_type::trivial_integer_type(uint16_t x) : m_value_unsigned(x), m_is_signed(false) {}
trivial_integer_type::trivial_integer_type(uint32_t x) : m_value_unsigned(x), m_is_signed(false) {}
trivial_integer_type::trivial_integer_type(uint64_t x) : m_value_unsigned(x), m_is_signed(false) {}



trivial_arithmetic_type::trivial_arithmetic_type(int8_t x) : m_value_signed(x), m_type(0) {}
trivial_arithmetic_type::trivial_arithmetic_type(int16_t x) : m_value_signed(x), m_type(0) {}
trivial_arithmetic_type::trivial_arithmetic_type(int32_t x) : m_value_signed(x), m_type(0) {}
trivial_arithmetic_type::trivial_arithmetic_type(int64_t x) : m_value_signed(x), m_type(0) {}

trivial_arithmetic_type::trivial_arithmetic_type(uint8_t x) : m_value_unsigned(x), m_type(1) {}
trivial_arithmetic_type::trivial_arithmetic_type(uint16_t x) : m_value_unsigned(x), m_type(1) {}
trivial_arithmetic_type::trivial_arithmetic_type(uint32_t x) : m_value_unsigned(x), m_type(1) {}
trivial_arithmetic_type::trivial_arithmetic_type(uint64_t x) : m_value_unsigned(x), m_type(1) {}

trivial_arithmetic_type::trivial_arithmetic_type(float x) : m_value_floating(x), m_type(2) {}
trivial_arithmetic_type::trivial_arithmetic_type(double x) : m_value_floating(x), m_type(2) {}
trivial_arithmetic_type::trivial_arithmetic_type(long double x) : m_value_floating(x), m_type(2) {}

trivial_arithmetic_type::trivial_arithmetic_type(trivial_integer_type x) : m_type(!x.m_is_signed), m_value_signed(x.m_value_signed) {}


_KSN_END
