
#include <iostream>
#include <fstream>
#include <string>

#define prime false

int main()
{
	static constexpr size_t top = 100'000'000;
	static constexpr size_t N = top + 1;
	using arr_t = uint8_t[N];

	std::ofstream fout("up to " + std::to_string(top) + ".txt");
	if (!fout) return -1;

	auto* parr = (arr_t*)new (std::nothrow) arr_t{};
	if (!parr) return -2;
	auto& arr = *parr;

	for (size_t i = 2; i < N; ++i)
	{
		if (arr[i] != prime)
			continue;
		for (size_t j = i + i; j < N; j += i)
			arr[j] = !prime;
	}
	size_t count = 0;
	for (size_t i = 2; i < N; ++i)
	{
		if (arr[i] == prime)
		{
			fout << i << '\n';
			++count;
		}
	}
	std::cout << "Found: " << count << "\n";
}
