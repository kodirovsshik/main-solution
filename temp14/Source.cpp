
#include <ksn/time.hpp>

#include <stdio.h>
#include <math.h>

#pragma comment(lib, "libksn_time")


uint64_t test()
{
	static const ksn::time sleep_time = ksn::time::from_usec(7000);

	ksn::stopwatch sw;
	ksn::time dt;

	sw.start();
	ksn::hybrid_sleep_for(sleep_time);
	dt = sw.stop();

	return dt.as_nsec();
}

int main()
{
	static constexpr size_t N = 100;

	ksn::init_hybrid_sleep_threshold();

	uint64_t tests[N];
	uint64_t avg = 0;
	uint64_t avg2 = 0;

	for (auto& entry : tests)
		avg += (entry = test());

	avg /= N;

	for (size_t i = 0; i < N; ++i)
	{
		uint64_t delta = tests[i] - avg;
		delta *= delta;

		avg2 += delta;
	}

	float derivation;
	derivation = sqrtf((float)avg2 / N);

	printf("%lli +- %g usec\n", avg / 1000, derivation / 1000);

	return 0;
}