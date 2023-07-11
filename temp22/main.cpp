
import std;

class A
{
	std::string s;

public:
	A(const std::string& s) : s(s) { std::print("{}\n", s); }
	~A() { std::print("~{}\n", s); }

	void foo() noexcept
	{
		std::print("Hi\n");
	}
};

int main()
{
	A a("a");
	if constexpr (noexcept(a.foo()))
		std::print("yes");
}
