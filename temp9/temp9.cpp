#include <stdio.h>
#include <iterator>
#include <algorithm>
#include <utility>
#include <numeric> //for iota
#include <random>

template<class Iterator, class Pred>
void merge_sorted(Iterator b1, Iterator b2, const Iterator e, Pred&& pred)
{
	Iterator b1p = b1;
	Iterator b2p = b2;
	b1p++; b2p++;

	while (b1p != b2 && b2p != e)
	{
		if (pred(*b2, *b1))
		{
			std::iter_swap(b1, b2);
			if (pred(*b2p, *b2))
			{
				b2 = b2p;
				++b2p;
			}
		}
		b1 = b1p;
		++b1p;
	}

	if (b1p == b2)
	{
		if (b2p == e)
		{
			//Two elements
			if (pred(*b2, *b1)) std::iter_swap(b1, b2);
			return;
		}
		while (b2 != e)
		{
			if (!pred(*b2, *b1))
			{
				std::rotate(b1, std::next(b1), b2);
				break;
			}
			++b2;
		}
	}
	else if (b2p == e)
	{
		while (b1 != b2)
		{
			if (pred(*b2, *b1))
			{
				std::rotate(b1, b2, std::next(b2));
				break;
			}
			++b1;
		}
	}
}


template<class Iterator, class Pred>
void merge_sort(Iterator b, Iterator e, Pred pred)
{
	const auto diff = std::distance(b, e);
	if (diff <= 1) return;
	if (diff == 2)
	{
		e = b;
		++e;
		if (pred(*e, *b)) std::iter_swap(b, e);
		return;
	}

	Iterator m = b;
	std::advance(m, diff / 2);

	merge_sort(b, m, pred);
	merge_sort(m, e, pred);
	merge_sorted(b, m, e, pred);
}

template<class Iterator>
void merge_sort(Iterator b, Iterator e)
{
	return merge_sort(b, e, std::less<>());
}
int main2();
int main()
{
	//main2();
	uint8_t arr[] = { 2, 3, 5, 7, 1, 8, 4, 6 };
	merge_sort(arr, arr + 8);

}

int main2()
{
	std::random_device rd;
	std::mt19937 engine(rd());
	

	constexpr int n = 16, N = 10000;
	int arr[n];
	for (int _ = 0; _ < 10000; ++_)
	{
		std::iota(arr, arr + n, 0); std::shuffle(arr, arr + n, engine);
		merge_sort(arr, arr + n);
		if (!std::is_sorted(arr, arr + n)) abort();
	}
	printf("Ok");
}