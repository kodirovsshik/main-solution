
#include <variant>

int main()
{
	std::variant<int, float> v;
	v = 42;

	auto&& x = std::get<int>(v);
}
