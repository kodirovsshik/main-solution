
#include <intrin.h>

#include <random>
#include <ranges>

#include <ksn/stuff.hpp>

#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_x86_instruction_set.lib")


//1 limb = 64 bytes

extern "C" void _mm_and_native(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_and_sse(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_and_avx(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_and_avx512(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);

void (*mm_and)(void*, const void*, const void*, size_t) = _mm_and_native;

void f(void* pdst, const void* psrc1, const void* psrc2, size_t limbs)
{
	uint64_t* d = (uint64_t*)pdst;
	uint64_t* s1 = (uint64_t*)psrc1;
	uint64_t* s2 = (uint64_t*)psrc2;
	uint64_t* e = d + limbs * 8;

	while (d != e)
	{
		*d++ = *s1++ & *s2++;
	}
}

int main1()
{
	int c = 0;
	int x1, x2;

	int a[96], b[96];
	for (int& x : a) x = c++;
	for (int& x : b) x = ~1;
	
	x1 = a[-1];
	x2 = a[96];
	_mm_and_avx512(a, a, b, 6);
	ksn_dynamic_assert(a[-1] == x1, "Memory corruption");
	ksn_dynamic_assert(a[96] == x2, "Memory corruption");
	
	c = 0;
	for (int x : a) printf("[%i]: %i\n", c++, x);
	(void)getchar();
	return 0;
}

constexpr static size_t limbs = 8;
int* p = (int*)malloc(limbs * 64);

float test()
{
	memset(p, 0, limbs * 64);

	auto d1 = ksn::measure_running_time_no_return(_mm_and_avx, p, p, p, limbs);
	auto d2 = ksn::measure_running_time_no_return(_mm_and_native, p, p, p, limbs);

	return (float(d2) - d1) / d2 * 100;
}

int main()
{
	static constexpr size_t tests = 200000;

	float data[tests];
	float avg = 0;
	int c = 0;
	for (auto& x : data)
	{
		printf("test %i/%i\n", ++c, (int)tests);
		do
		{
			x = test();
		} while (fpclassify(x) != FP_NORMAL);
		avg += x;
	}
	avg /= tests;

	float avg2 = 0;
	for (auto x : data)
	{
		float t = x - avg;
		avg2 += t * t;
	}
	avg2 = sqrtf(avg2 / tests);

	printf("%g +- %g (e = %.1f%%)\n", avg, avg2, avg2 / fabs(avg) * 100);

	(void)getchar();
	return 0;
}
