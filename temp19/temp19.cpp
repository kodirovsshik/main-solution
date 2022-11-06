
#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>
#include <variant>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>

#include <ctype.h>
#include <stdint.h>

using namespace std;

namespace ksn
{
	template<class Iter, class Pred>
	Iter linear_stable_partition(Iter begin, Iter end, Pred&& pred)
	{
		Iter bound = begin, runner;
		while (bound < end && pred(*bound))
			++bound;
		runner = bound;

	}
}

int main()
{

}
