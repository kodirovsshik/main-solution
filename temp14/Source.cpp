
#include <iostream>
#include <vector>
#include <algorithm>

#include <stdio.h>

#include <ksn/math_long_integer.hpp>


#pragma warning(disable : 4996)

int main()
{
	//(void)freopen("data.txt", "r", stdin);
	//(void)freopen("answer.txt", "wb", stdout);

	std::ios::sync_with_stdio(true);

	using T = int;

	T sum = 0;

	size_t N;
	std::cin >> N;

	std::vector<T> v;
	v.reserve(N);

	for (size_t i = 0; i < N; ++i)
	{
		T x;
		std::cin >> x;

		sum += x;
		v.push_back(x);
	}




	for (int n = N; n --> 0;)
	{
		trues += test();
	}

	fclose(stdout);
	fclose(stdin);

	return 0;
}