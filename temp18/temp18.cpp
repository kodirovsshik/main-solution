
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <climits>
using namespace std;

using kis = std::vector<uint8_t>;

uint8_t fdist(uint8_t a, uint8_t b)
{
	uint8_t x = b > a ? b - a : a - b;
	return min<uint8_t>(x, 4 - x);
}

int fdist(const kis& a, const kis& b)
{
	int d = 0;
	for (int i = 0; i < a.size(); ++i)
		d += fdist(a[i], b[i]);
	return d;
}

uint8_t mapc2i(char c)
{
	switch (c)
	{
	case 'A': return 0;
	case 'C': return 1;
	case 'G': return 2;
	case 'T': return 3;
	default: exit(-1);
	}
}

int main()
{
	int n, l, w = 0;
	cin >> n >> l;

	vector<uint8_t> done(n + 1, 0);
	done[0] = true;

	vector<kis> vkis;
	vector<int> dist(n + 1);
	vkis.reserve(n + 1);
	vkis.emplace_back(l, 0);

	for (int i = 1; i <= n; ++i)
	{
		string ks;
		cin >> ks;

		kis k(l);
		for (int i = 0; i < l; ++i)
			k[i] = mapc2i(ks[i]);
		dist[i] = fdist(k, vkis[0]);
		vkis.push_back(move(k));

	}

	int left = n;
	while (left > 0)
	{
		int min_i = -1;
		int min_v = INT_MAX;

		for (int i = 1; i <= n; ++i)
		{
			if (done[i])
				continue;
			//build ith
			if (dist[i] < min_v)
			{
				min_v = dist[i];
				min_i = i;
			}
		}

		done[min_i] = true;
		w += min_v;

		//recalculate dists
		for (int i = 1; i <= n; ++i)
		{
			if (done[i])
				continue;
			dist[i] = min(dist[i], fdist(vkis[i], vkis[min_i]));
		}
		--left;
	}

	cout << w;
}