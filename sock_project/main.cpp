#include <Windows.h>
#include <stdexcept>


int main()
{
	int core_wrapper();
	__try
	{
		return core_wrapper();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[Startup] CRITICAL: Unhandled SEH exception\n%i", GetExceptionCode());
	}
}

int core_wrapper()
{
	int invoke_core();
	try
	{
		return invoke_core();
	}
	catch (const std::exception& excp)
	{
		printf("[Startup] CRITICAL: Unhandled STD exception\n%s", excp.what());
	}
	return -1;
}

int invoke_core()
{
	int core(int, char**);
	return core(__argc, __argv);
}