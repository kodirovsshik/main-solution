
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#pragma warning(disable : 4996)

int main()
{
	FILE* f = fopen("data 5ms.bin", "rb");
	if (!f) return 1;

	fseek(f, 0, SEEK_END);
	auto sz = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (sz % 4) return 2;

	float* p = (float*)malloc(sz);
	if (!p) return 3;
	
	fread(p, 1, sz, f);
	int N = sz / sizeof(float);
	float avg = 0;
	for (int i = 0; i < N; ++i)
		avg += p[i] / N;
	
	float stddev = 0;
	for (int i = 0; i < N; ++i)
		stddev += powf(p[i] - avg, 2) / N;
	stddev = sqrtf(stddev);

	float c1{}, c2{}, c3{};

	for (int i = 0; i < N; ++i)
	{
		if (fabsf(p[i] - avg) > stddev)
			c1++;
		if (fabsf(p[i] - avg) > 2 * stddev)
			c2++;
		if (fabsf(p[i] - avg) > 3 * stddev)
			c3++;
	}
	printf("%f\n", stddev);
	printf("1: %f\n", (1 - c1 / N) * 100);
	printf("2: %f\n", (1 - c2 / N) * 100);
	printf("3: %f\n", (1 - c3 / N) * 100);
}