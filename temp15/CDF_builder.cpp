
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <algorithm>

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

	std::sort(p, p + N);

	float x1 = 0.0045f;
	float x2 = 0.0063f;
	static constexpr size_t n = 100;
	float dx = (x2 - x1) / n;

	float cdf[n]{};

	FILE* fcdf = fopen("cdf.txt", "wb");
	FILE* fpdf = fopen("pdf.txt", "wb");

	while (true)
	{
		static size_t i = 0;
		static size_t j = 0;
		static size_t j0 = 0;
		static float x = x1;
		//static float y = 0;
		while (j < N && p[j] <= x)
			++j;
		fprintf(fcdf, "%f, %f\n", x, (float)j / N);
		//fprintf(fpdf, "%f, %f\n", x, (float)(j - j0) / N / dx);
		j0 = j;
		x += dx;
		if (x >= x2)
			break;
	}
}