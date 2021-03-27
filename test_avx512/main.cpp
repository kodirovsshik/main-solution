
#include <intrin.h>

#include <random>
#include <ranges>

#include <ksn/stuff.hpp>
#include <ksn/x86_instruction_set.hpp>

#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_x86_instruction_set.lib")


//1 limb = 64 bytes

extern "C" void _mm_and_native(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_and_sse(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_and_avx(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_and_avx512(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);

extern "C" void _mm_xor_native(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_xor_sse(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_xor_avx(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_xor_avx512(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);

extern "C" void _mm_or_native(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_or_sse(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_or_avx(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);
extern "C" void _mm_or_avx512(void* pdst, const void* psrc1, const void* psrc2, size_t limbs);

void (*mm_and)(void*, const void*, const void*, size_t) = _mm_and_native;
void (*mm_or)(void*, const void*, const void*, size_t) = _mm_or_native;
void (*mm_xor)(void*, const void*, const void*, size_t) = _mm_xor_native;

int _initializer = []()->int
{
	if (ksn::x86_features->avx512_f)
	{
		mm_and = _mm_and_avx512;
		mm_xor = _mm_xor_avx512;
		mm_or = _mm_or_avx512;
	}
	else if (ksn::x86_features->avx)
	{
		mm_and = _mm_and_avx;
		mm_xor = _mm_xor_avx;
		mm_or = _mm_or_avx;
	}
	else if (ksn::x86_features->sse)
	{
		mm_and = _mm_and_sse;
		mm_xor = _mm_xor_sse;
		mm_or = _mm_or_sse;
	}

	return 0;
}();

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

constexpr static size_t limbs = 1024;
int* p = (int*)malloc(limbs * 64);

float test()
{
	memset(p, 0, limbs * 64);

	auto d1 = ksn::measure_running_time_no_return(_mm_and_avx, p, p, p, limbs);
	auto d2 = ksn::measure_running_time_no_return(_mm_and_sse, p, p, p, limbs);

	return (float(d2) - d1) / d2 * 100;
}

template<class fp_t>
fp_t dispersion(fp_t* data, size_t N, fp_t* pavg = nullptr)
{
	fp_t _avg = 0;
	if (pavg == nullptr)
	{
		pavg = &_avg;
		for (size_t i = 0; i < N; ++i)
		{
			*pavg += data[i];
		}
		*pavg /= N;
	}
	fp_t avg_diff = 0;
	fp_t t;
	for (size_t i = 0; i < N; ++i)
	{
		t = data[i] - *pavg;
		t *= t;
		avg_diff += t;
	}
	avg_diff /= N;
	return avg_diff;
}

template<class fp_t, size_t N>
fp_t dispersion(fp_t(&data)[N], fp_t* avg = nullptr)
{
	return dispersion(data, N);
}

template<class fp_t>
void estimate_error(fp_t* data, size_t N, fp_t* pavg = nullptr, fp_t* psigma = nullptr, fp_t* pepsilon = nullptr)
{
	using std::sqrt;

	if (pavg || psigma || pepsilon)
	{
		fp_t avg = 0;
		for (size_t i = 0; i < N; ++i)
		{
			avg += data[i];
		}
		avg /= N;
		if (pavg) *pavg = avg;
		
		if (psigma || pepsilon)
		{
			fp_t sigma = sqrt(dispersion(data, N, &avg));
			if (psigma) *psigma = sigma;
			if (pepsilon) *pepsilon = sigma / avg;
		}
	}
}

template<class fp_t, size_t N>
void estimate_error(fp_t* data, fp_t* pavg = nullptr, fp_t* psigma = nullptr, fp_t* pepsilon = nullptr)
{
	return estimate_error(data, N, pavg, psigma, pepsilon);
}

template<class fp_t>
void print_measurement_result(fp_t* data, size_t N)
{
	fp_t avg, sigma, epsilon;
	estimate_error(data, N, &avg, &sigma, &epsilon);
	printf("%g +- %g (e = %g%%)\n", float(avg), float(sigma), float(epsilon * fp_t(100)));
}

int main()
{
	static constexpr size_t tests = 100000;
	static constexpr size_t tests_discard = 10000;
	float data[tests];
	float data_discarded[tests_discard];
	int c = 0;

	memset(p, 0, limbs * 64);

	printf("Running discarded tests\n");
	for (size_t i = 0; i < tests_discard;)
	{
		++i;
		printf("Test %zu/%zu\n", i, tests_discard);
		float x;
		do
		{
			x = test();
		} while (fpclassify(x) != FP_NORMAL);
		data_discarded[i - 1] = x;
	}

	printf("Running actual tests\n");
	for (auto& x : data)
	{
		printf("Test %i/%i\n", ++c, (int)tests);
		do
		{
			x = test();
		} while (fpclassify(x) != FP_NORMAL);
	}

	printf("Results:\n");
	print_measurement_result(data, tests);
	printf("Discarded:\n");
	print_measurement_result(data_discarded, tests_discard);

	(void)getchar();
	return 0;
}
