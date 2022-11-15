

constexpr int foo()
{
	if consteval
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int main()
{
	return foo();
}