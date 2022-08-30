
#include <stdlib.h>
#include <math.h>

float randf()
{
	return rand() / (float)RAND_MAX;
}

int main()
{
	const size_t N = 16;
	float noise[N]{};

	const float amplitude = 1;
	const size_t steps = 8;

	float current_amplitude = amplitude / 2;

	for (size_t n = 0; n < steps; ++n)
	{
		for (auto& x : noise)
			x += randf() * current_amplitude;
		current_amplitude /= 2;
	} 

	float avg = 0;
	for (auto x : noise)
		avg += x;
	avg /= N;

	const float variance_expected = amplitude * amplitude / 12;
	float variance = 0;
	for (auto x : noise)
	{
		float diff = x - avg;
		variance += diff * diff;
	}
	variance /= N;

	[] {}();
}