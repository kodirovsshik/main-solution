
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
//#include <numeric>
//#include <set>
//#include <unordered_set>
//#include <map>
//#include <unordered_map>

using namespace std;

static const auto _____ = []()
{
	// fast IO
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	cout.tie(nullptr);
	return 0;
}();


class Solution {
public:
	int earliestFullBloom(vector<int>& plantTime, vector<int>& growTime) {
		const int n = plantTime.size();

		associated_sort(
			std::tuple{ plantTime.begin(), growTime.begin() },
			std::tuple{ plantTime.end(), growTime.end() },
			[&](auto&& pl1, auto&& pl2)
		{
			const int p1 = pl1.get<0>();
			const int g1 = pl1.get<1>();
			const int p2 = pl2.get<0>();
			const int g2 = pl2.get<1>();
			const int x1 = p1 + max(g1, p2 + g2);
			const int x2 = p2 + max(g2, p1 + g1);
			return x1 < x2;
		});
		
		int plant_time = 0;
		int max_time = 0;
		for (size_t k = 0; k < n; ++k)
		{
			plant_time += plantTime[k];
			max_time = max(max_time, plant_time + growTime[k]);
		}
		return max_time;
	}
};

int main()
{
	vector<int> v1{ 4,3,1 }, v2{ 1,2,3 };
	Solution().earliestFullBloom(v1, v2);

	float(*(*ptr)(int))() = 0;

	return 0;
}
