
#define task_main __task_main
#define main task_main





#include <iostream>
#include <vector>
#include <iterator>
#include <numeric>
#include <ranges>
#include <algorithm>

using T = int;
static constexpr T INF = std::numeric_limits<T>::max();


struct solve_result
{
	std::vector<T> v1, v2;
};

T get(const std::vector<T>& v, size_t n)
{
	return v.size() > n ? v[n] : 0;
}

T xabs(T x)
{
	return x >= 0 ? x : -x;
}

solve_result try_solve(const std::vector<T>& v, size_t n1)
{
	const size_t n2 = v.size() - n1;

	solve_result r;
	r.v1.reserve(n1);
	r.v2.reserve(n2);
	std::copy(v.begin(), v.begin() + n1, std::back_inserter(r.v1));
	std::copy(v.begin() + n1, v.end(), std::back_inserter(r.v2));

	T s1, s2;
	s1 = std::accumulate(r.v1.begin(), r.v1.end(), 0);
	s2 = std::accumulate(r.v2.begin(), r.v2.end(), 0);
	
	for (size_t i = 0; i < n2; ++i)
	{
		for (size_t j = 0; j < n1; ++j)
		{
			T b, c;
			b = get(r.v2, i);
			c = get(r.v1, j);
			if (c == 0)
				break;
			T y = c - b;
			if (xabs(s1 - y - (s2 + y)) < xabs(s1 - s2))
			{
				std::swap(r.v1[j], r.v2[i]);
				s1 -= y;
				s2 += y;
			}
		}
	}

	return r;
}

int main_my()
{
	solve_result best_result = {};
	T best_value = INF;

	size_t n;
	std::cin >> n;

	std::vector<T> v;
	v.reserve(n);

	for (size_t i = 0; i < n; ++i)
	{
		T x;
		std::cin >> x;
		if (x == 0)
			continue;
		v.push_back(x);
	}

	n = v.size();
	for (size_t i = 0; i < n; ++i)
	{
		solve_result res = try_solve(v, i);

		T s1, s2;
		s1 = std::accumulate(res.v1.begin(), res.v1.end(), 0);
		s2 = std::accumulate(res.v2.begin(), res.v2.end(), 0);
		T value = xabs(s1 - s2);
		if (value < best_value)
		{
			best_result = res;
			best_value = value;
		}
	}

	std::copy(best_result.v1.begin(), best_result.v1.end(), std::ostream_iterator<T>(std::cout, " "));
	std::cout << "\n";
	std::copy(best_result.v2.begin(), best_result.v2.end(), std::ostream_iterator<T>(std::cout, " "));

	return 0;
}


int main()
{
	size_t n;
	std::cin >> n;

	std::vector<int> v;
	v.reserve(n);

	std::copy(std::istream_iterator<int>(std::cin), std::istream_iterator<int>(), std::back_inserter(v));



	return 0;
}





#undef main

#include <sstream>
#include <fstream>
#include <string>
#include <unordered_set>

int sum_up(const std::string& s)
{
	std::istringstream ss(s);
	int sum = 0;
	while (!ss.eof())
	{
		int x;
		ss >> x;
		if (!ss)
			break;
		sum += x;
	}
	return sum;
}

void record_create(std::unordered_set<int>& db, const std::string& data)
{
	db.clear();
	std::istringstream ss(data);
	int n;
	ss >> n;
	while (n --> 0)
	{
		int x;
		ss >> x;
		if (!ss)
			throw;
		db.insert(x);
	}
}

bool record_test(std::unordered_set<int>& db, const std::string& data)
{
	std::istringstream ss(data);
	while (true)
	{
		int x;
		ss >> x;
		if (!ss)
			break;
		if (!db.contains(x))
			return false;
		db.erase(x);
	}
	return true;
}

int diff(int a, int b)
{
	return a >= b ? a - b : b - a;
}

int main()
{
	std::string in;
	std::string out1_best;
	std::string out2_best;

	std::string out1_test;
	std::string out2_test;

	std::ifstream test_data("tests_random.txt");

	std::unordered_set<int> values;
	
	auto cin_rdbuf = std::cin.rdbuf();
	auto cout_rdbuf = std::cout.rdbuf();

	float score = 0;
	int test_n = 0;

	while (!test_data.eof())
	{
		std::getline(test_data, in);
		std::getline(test_data, out1_best);
		std::getline(test_data, out2_best);

		if (in.empty())
			break;

		if (!test_data)
		{
			std::cerr << "Test " << test_n << ": Invalid input data\n";
			return test_n;
		}

		++test_n;
	
		std::istringstream iss(in);
		std::cin.set_rdbuf(iss.rdbuf());
		std::cin.seekg(0, std::ios::beg);

		std::stringstream ss;
		std::cout.set_rdbuf(ss.rdbuf());
		std::cout.seekp(0, std::ios::beg);

		task_main();

		std::getline(ss, out1_test);
		std::getline(ss, out2_test);

		record_create(values, in);
		if (!record_test(values, out1_test) || !record_test(values, out2_test) || values.size() != 0)
		{
			std::cerr << "Test " << test_n << ": WA\n";
			__debugbreak();
		}
		else
		{
			const int best1 = sum_up(out1_best);
			const int best2 = sum_up(out2_best);
			const int best_diff = diff(best1, best2);

			const int test1 = sum_up(out1_test);
			const int test2 = sum_up(out2_test);
			const int test_diff = diff(test1, test2);

			score += 100 * expf(diff(test_diff, best_diff) / -4.f);
		}
	}

	score /= test_n;

	std::cin.set_rdbuf(cin_rdbuf);
	std::cout.set_rdbuf(cout_rdbuf);

	std::cout << "Score: " << score << std::endl;

	return 0;
}
