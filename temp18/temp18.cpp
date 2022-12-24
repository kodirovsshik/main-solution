
#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <queue>
#include <deque>
#include <algorithm>
#include <numeric>
#include <functional>
#include <variant>
#include <random>
#include <charconv>

#include <ctype.h>
#include <stdint.h>

#pragma warning(disable : 4068)
using namespace std;


#pragma GCC optimize("O3")
auto fast_io = []
{
	ios_base::sync_with_stdio(false);
	cin.tie(0);
	cout.tie(0);
	return 0;
}();


class Solution
{
	const char* end;

#define skip_whitespace(p) while (*p == ' ') { ++p; }

	bool try_parse_num(const char* &ptr, int& result)
	{
		const char* p = ptr;
		skip_whitespace(p);
		if (*p == '+') ++p;
		else if (*p == '-')
		{
			++p;
			if (try_parse_num(p, result))
			{
				ptr = p;
				result = -result;
				return true;
			}
			return false;
		}

		skip_whitespace(p);

		auto res = from_chars(p, end, result);
		if (res.ec != errc{})
			return false;

		ptr = res.ptr;
		return true;
	}
	bool try_parse_expr(const char*& ptr, int& result)
	{
		const char* p = ptr;
		skip_whitespace(p);

		if (*p == '+')
			++p;
		else if (*p == '-')
		{
			++p;
			skip_whitespace(p);
			if (try_parse_expr(p, result))
			{
				result = -result;
				ptr = p;
				return true;
			}
			return false;
		}

		skip_whitespace(p);
		if (*p != '(')
			return false;
		++p;

		int x;
		if (parse_sum(p, x))
		{
			skip_whitespace(p);
			if (*p != ')')
				return false;
			ptr = p + 1;
			result = x;
			return true;
		}
		return false;
	}
	bool parse_sum(const char* &ptr, int& result)
	{
		int sum = 0;
		while (true)
		{
			int x;
			if (try_parse_num(ptr, x))
				sum += x;
			else if (try_parse_expr(ptr, x))
				sum += x;
			else
			{
				result = sum;
				return true;
			}
		}
	}
public:
	int calculate(const string& s)
	{
		const char* p = s.data();
		end = p + s.size();
		int x;
		parse_sum(p, x);
		return x;
	}
};

template<class T, class... Args>
void test(T ans, const Args& ...args)
{
	if (Solution().calculate(args...) != ans)
		__debugbreak();
}

int main()
{
	test(6, "1 + 2 + 3");
	test(0, "1 + 2 - 3");
	test(2, "1 - 2 + 3");
	test(-4, "1 - 2 - 3");
	test(-4, "1 - (2 + 3)");
	test(2, "1 - (2 - 3)");
	test(6, "1 + (2 + 3)");
	test(2, "1 + (-2 + 3)");
	test(3, "   (  3 ) ");
}
