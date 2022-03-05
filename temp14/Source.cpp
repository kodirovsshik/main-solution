
#include <string>
#include <algorithm>
#include <vector>
#include <fstream>
#include <numeric>

struct record
{
	std::string name;
	int price;
	int sold;

	friend bool operator<(const record& a, const record& b)
	{
		return (uint64_t)a < (uint64_t)b;
	}

	friend std::istream& operator>>(std::istream& is, record& r)
	{
		std::getline(is, r.name, ',');
		is >> r.price >> r.sold;
		return is;
	}
	friend std::ostream& operator<<(std::ostream& os, const record& r)
	{
		os << r.name;// << ", " << (uint64_t)r;
		return os;
	}

	operator uint64_t() const
	{
		return uint64_t(this->price);
	}
};

int main()
{
	std::vector<record> records;
	std::ifstream fin("input.csv");
	std::ofstream fout("output2.csv");

	if (!fin) return 1;

	while (!fin.eof())
	{
		record r;
		fin >> r;
		records.push_back(std::move(r));
		fin.ignore(LLONG_MAX, '\n');
	}

	std::sort(records.rbegin(), records.rend());
	const auto total_cost = std::accumulate(records.begin(), records.end(), (uint64_t)0);


	std::vector<record> groups[3];
	uint64_t thresholds[3] = { total_cost * 8 / 10, total_cost * 95 / 100, total_cost };

	size_t current_element = 0;
	uint64_t current_total = 0;
	for (size_t i = 0; i < std::size(groups); ++i)
	{
		while (current_element < records.size())
		{
			auto& elem = records[current_element];
			if (current_total + elem > thresholds[i])
				break;
			++current_element;
			current_total += elem;
			groups[i].push_back(std::move(elem));
		}

		if (current_element == records.size())
			break;
	}

	for (const auto& group : groups)
	{
		fout << "(Group boundrary)\n";
		for (const auto& elem : group)
			fout << elem << std::endl;
		fout << std::endl;
	}
	return 0;
}
