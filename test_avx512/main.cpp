
#include <intrin.h>

#include <random>



int main()
{

	__m512 x;
	__m512i& x_i = *(__m512i*)&x;

	for (float& e : x.m512_f32) e = float(&e - x.m512_f32) + 0.5f;

	x = _mm512_castsi512_ps(_mm512_cvt_roundps_epi32(x, _MM_ROUND_MODE_NEAREST));
	
}
