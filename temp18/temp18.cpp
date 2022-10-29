
#include <iostream>

#include <experimental/generator>
#include <coroutine>

#include <ksn/debug_utils.hpp>
#pragma comment(lib, "libksn_debug_utils")

template<class T, class Alloc = std::allocator<T>>
using generator = std::experimental::generator<T, Alloc>;

template<class T>
generator<T> range(T end, T begin = T(0), T step = T(1))
{
	int _;
	ksn::copy_debugger sentry;
	while (true)
	{
		_ = 1;
		co_yield 1;
		_ = 2;
	}
}

int main()
{
	auto co = range(3);
	auto gen = co.begin();

	++gen;
}