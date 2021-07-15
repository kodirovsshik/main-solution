#pragma once

#ifndef _KSN_INTEGER_IMPL_DEFINES_HPP_
#define _KSN_INTEGER_IMPL_DEFINES_HPP_



#define BYTES_PER_UNIT sizeof(uint64_t)
#define BITS_PER_UNIT (CHAR_BIT * BYTES_PER_UNIT)

#define construct_if_empty(this) if ((this)->m_capacity == 0) { (this)->_first_init(linteger::g_s_initial_capacity == 0 ? 1 : linteger::g_s_initial_capacity); } ((void)0)

#define trivial_ar_to_i64(x) (((x).m_type == 2) ? int64_t((x).m_value_floating) : (x).m_value_signed)

#define set_result_flags(result, a, b) \
result.g_capacity_flags = a.g_capacity_flags & b.g_capacity_flags;\
result.g_flags.autoextend_self = result.g_flags.autoextend_self; ((void)0)


#define _adc32 _addcarry_u32
#define _adc64 _addcarry_u64

#define _sbb32 _subborrow_u32
#define _sbb64 _subborrow_u64

#define _imul32 _emul
#define _mul32 _emulu

#define _imul64 _mul128
#define _mul64 _umul128

#define _imulh __mulh
#define _mulh __umulh


//
//#define g_autoshrink g_flags.autoshrink
//#define g_autoextend_self g_flags.autoextend_self
//#define g_autoextend_result g_flags.autoextend_result
//


#endif //_KSN_INTEGER_IMPL_DEFINES_HPP_
