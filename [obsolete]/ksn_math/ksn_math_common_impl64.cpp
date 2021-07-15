// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <ksn/math_common.hpp>

_KSN_BEGIN



_KSN_DETAIL_BEGIN

extern const signed char _log2_lookup_table8[256];

_KSN_DETAIL_END



size_t log2_64(uint64_t x)
{
	uint64_t t1, t2;

	if ((t1 = (x >> 32)))
	{
		if ((t2 = (t1 >> 16)))
		{
			return (t1 = (t2 >> 8)) ? (32 + 16 + 8 + detail::_log2_lookup_table8[t1]) : (32 + 16 + detail::_log2_lookup_table8[t2]);
		}
		else
		{
			return (t2 = (t1 >> 8)) ? (32 + 8 + detail::_log2_lookup_table8[t2]) : (32 + detail::_log2_lookup_table8[t1]);
		}
	}
	else
	{
		if ((t2 = (x >> 16)))
		{
			return (t1 = (t2 >> 8)) ? (16 + 8 + detail::_log2_lookup_table8[t1]) : (16 + detail::_log2_lookup_table8[t2]);
		}
		else
		{
			return (t1 = (x >> 8)) ? (8 + detail::_log2_lookup_table8[t1]) : detail::_log2_lookup_table8[x];
		}
	}
}


_KSN_END
