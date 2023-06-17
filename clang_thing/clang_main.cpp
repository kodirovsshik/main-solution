
//#define MDSPAN_USE_PAREN_OPERATOR 1
#include <mdspan.hpp>

int main()
{
	namespace stdex = std::experimental;
	size_t n = 6, m = 3;
	std::vector<int> v(n*m);
	using exts = stdex::dextents<size_t, 2>;

	stdex::mdspan<int, exts> span(v.data(), exts(n, m));
	span[4, 2] = 42;
}