
#include <intrin.h>

double& at(__m128d& x, size_t at)
{
	return x.m128d_f64[at];
}

int main()
{

	__m128d x;
	at(x, 0) = 0;
	at(x, 1) = 1;
	x = _mm_castps_pd(_mm_asin_ps(_mm_castpd_ps(x)));
}
