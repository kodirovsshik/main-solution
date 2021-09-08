
#include <cmath>
#include <limits.h>

#include <iostream>
#include <random>


float get_rand(float max)
{
	static std::uniform_real_distribution<float> distribution(0, 1);
	static std::mt19937_64 random_engine;

	return distribution(random_engine) * max;
}

bool test()
{
	constexpr static float dt1 = 20, dt2 = 5, dt = 60;

	float t1 = get_rand(dt);
	float t2 = get_rand(dt);

	return t2 + dt2 >= t1 && t2 <= t1 + dt1;
}


int main()
{

	static constexpr int N = 100000000;
	int trues = 0;

	for (int n = N; n --> 0;)
	{
		trues += test();
	}

	std::cout << trues << "/" << N << "\n" << double(trues) / N;
}
