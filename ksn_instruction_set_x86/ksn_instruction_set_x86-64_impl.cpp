// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com



#include <ksn/ksn.hpp>
#include <ksn/instruction_set_x86-64.hpp>

#include <immintrin.h>

#include <string.h>

#if defined _KSN_COMPILER_GCC || defined _KSN_COMPILER_CLANG
#include <cpuid.h>
#endif

#ifdef _KSN_COMPILER_MSVC
#include <intrin.h>
#endif


extern "C" static int __ksn_get_cpuid_available_i386()
{
#if defined _KSN_COMPILER_GCC || defined _KSN_COMPILER_CLANG
	int __cpuid_supported = 0;

	__asm(
	"  pushfl\n"
	"  popl   %%eax\n"
	"  movl   %%eax,%%ecx\n"
	"  xorl   $0x00200000,%%eax\n"
	"  pushl  %%eax\n"
	"  popfl\n"
	"  pushfl\n"
	"  popl   %%eax\n"
	"  movl   $0,%0\n"
	"  cmpl   %%eax,%%ecx\n"
	"  je     1f\n"
	"  movl   $1,%0\n"
	"1:"
	: "=r" (__cpuid_supported) : : "eax", "ecx"
	);

	return __cpuid_supported;

#elif defined _KSN_COMPILER_MSVC

	if _KSN_CONSTEXPR_CONDITION(sizeof(void*) == 8)
	{
		return 1;
	}
	else
	{
		static const _KSN_CONSTEXPR size_t id_flag = 1 << 21;
		
		auto eflags_expected = __readeflags();
		eflags_expected ^= id_flag;
		__writeeflags(eflags_expected);

		auto eflags_got = __readeflags();

		eflags_expected &= id_flag;
		eflags_got &= id_flag;

		return eflags_got == eflags_expected;
	}
}
#else
{
	return 0;
}
#endif
;


_KSN_BEGIN

_KSN_DETAIL_BEGIN



static uint32_t mask_1c = uint32_t(-1);
static uint32_t mask_1d = uint32_t(-1);
static uint32_t mask_7_0b = uint32_t(-1);
static uint32_t mask_7_0c = uint32_t(-1);
static uint32_t mask_7_0d = uint32_t(-1);
static uint32_t mask_7_1a = uint32_t(-1);
static uint32_t mask_13_1a = uint32_t(-1);
static uint32_t mask_14a = uint32_t(-1);

//extended

static uint32_t mask_1ec = uint32_t(-1);
static uint32_t mask_1ed = uint32_t(-1);



static bool _cpuid_supported;

static __ksn_x86_64_features __features;

static bool _KSN_FORCEINLINE init_feature_struct1(detail::__ksn_x86_64_features* pf)
{
	if (!_cpuid_supported)
	{
		memset(pf, 0, sizeof(*pf));
		return false;
	}

	uint32_t eax, ebx, ecx, edx;
	uint32_t *p = (uint32_t*)pf;

	uint32_t max;
	cpuid(0, eax, ebx, ecx, edx);
	max = eax;


	if (max < 1) goto extended;

	cpuid(1, eax, ebx, ecx, edx);
	*p++ = edx & mask_1d;
	*p++ = ecx & mask_1c;


	if (max < 7) goto extended;

	cpuid(7, eax, ebx, ecx, edx, 0);
	*p++ = ebx;
	*p++ = ecx;
	*p++ = edx;

	if (eax >= 1)
	{
		cpuid(7, eax, ebx, ecx, edx, 1);
		*p++ = eax;
	}
	else
	{
		*p++ = 0;
	}


	if (max < 13) goto extended;

	cpuid(13, eax, ebx, ecx, edx, 1);
	*p++ = eax;


	if (max < 14) goto extended;

	cpuid(14, eax, ebx, ecx, edx);
	*p++ = eax;


extended:
;
	uint32_t *pe = (uint32_t*)x86_features + 8;
	while (p != pe)
	{
		*p++ = 0;
	}

	cpuid(0x80000000, eax, ebx, ecx, edx);
	max = eax;


	if (max < 0x80000001) return true;

	cpuid(0x80000001, eax, ebx, ecx, edx);
	*p++ = ecx;
	*p++ = edx;

	return true;
}

static int initializer()
{
	if _KSN_CONSTEXPR_CONDITION (sizeof(void*) == 4)
	{
		_cpuid_supported = __ksn_get_cpuid_available_i386();

		if (!_cpuid_supported)
		{
			mask_1c = 0;
			mask_1d = 0;
			mask_7_0b = 0;
			mask_7_0c = 0;
			mask_7_0d = 0;
			mask_7_1a = 0;
			mask_13_1a = 0;
			mask_14a = 0;
			return -1;
		}
	}
	else
	{
		_cpuid_supported = true;
	}

	bool apply_amd_mask = false;
	bool apply_intel_mask = false;

	struct
	{
		int eax;
		int ebx;
		int ecx;
		int edx;
	} registers;

	cpuid(0, (uint32_t*)&registers);

	//Conversion of strings to these sigs myself was a bit painful :\

	if ((	registers.ebx == 0x68747541 && //"AuthenticAMD"
		registers.edx == 0x69746E65 &&
		registers.ecx == 0x444D4163 ) || (
		registers.ebx == 0x69444D41 && //"AMDisbetter!" //yes, some of AMD CPUs (rarely) implement this :p
		registers.edx == 0x74656273 &&
		registers.ecx == 0x21726574 ))
	{
		apply_amd_mask = true;
	}
	else if (
		registers.ebx == 0x756E6547 && //"GenuineIntel"
		registers.edx == 0x49656E69 &&
		registers.ecx == 0x6C65746E )
	{
		apply_intel_mask = true;
	}

	if (apply_intel_mask)
	{
		mask_1c &= 0b11111111111111101111111111111111;
		mask_1d &= 0b11111111111011111111101111111111;
		mask_7_0b &= ~(uint32_t(1) << 6);
		mask_7_0c &= 0b01011010011111110101111111111111;
		mask_7_0d &= 0b10101100000101000000000000011100;
		mask_7_1a &= uint32_t(1) << 5;
	}
	if (apply_amd_mask)
	{
		mask_1c &= 0b11111110100110000011001000001011;
		mask_1d &= 0b00010111100010111111101111111111;
	}

	return init_feature_struct1((__ksn_x86_64_features*)x86_features) - 1;
}

static int constructor_caller = initializer();

_KSN_DETAIL_END



const detail::__ksn_x86_64_features* x86_features = &detail::__features;



bool cpuid(int l, uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, int s) noexcept
{
	if (!detail::_cpuid_supported)
	{
		return false;
	}
#if defined _KSN_COMPILER_MSVC
	int arr[4];
	__cpuidex(arr, l, s);
	a = arr[0];
	b = arr[1];
	c = arr[2];
	d = arr[3];
#elif defined __GNUC__
	__cpuid_count(l, s, a, b, c , d);
#else
	return false;
#endif
	return true;
}

bool cpuid(int l, uint32_t* p, int s) noexcept
{
	if (!detail::_cpuid_supported)
	{
		return false;
	}
#if defined _KSN_COMPILER_MSVC
	__cpuidex((int*)p, l, s);
#elif defined __GNUC__
	__cpuid_count(l, s, p[0], p[1], p[2], p[3]);
#else
	return false;
#endif
	return true;
}

bool cpuid_supported() noexcept
{
	return detail::_cpuid_supported;
}

bool init_feature_struct(detail::__ksn_x86_64_features* p) noexcept
{
	if ((void*)p == (void*)x86_features)
	{
		return false;
	}
	return detail::init_feature_struct1(p);
}


_KSN_END
