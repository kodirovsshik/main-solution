#include <stdio.h>
#include <Windows.h>

#pragma warning(disable : 4996)

extern "C" size_t __cdecl get_base_pointer();
extern "C" size_t stacktrace_frames(void** ptrs_buffer, size_t entries_limit)
{
	void _stacktrace1(void**, size_t, size_t*); //Get frame pointers

	size_t result;
	__try
	{
		_stacktrace1(ptrs_buffer, entries_limit, &result);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
	return result;
}


static void* buffer[256];
size_t num_entries;

void recursion(unsigned level = 0)
{
	if (level == 0)
	{
		num_entries = stacktrace_frames(buffer, 256);
		return;
	}

	printf("Entered recursion level %i [%0*zX]\n", level, sizeof(void*) * 2, get_base_pointer());
	recursion(level - 1);
	printf("Leaving recursion level %i [%0*zX]\n", level, sizeof(void*) * 2, get_base_pointer());
}

int main()
{
	int n, _;
	do
	{
		printf("Enter recursion levels: ");
		_ = scanf("%i", &n);
	} while (_ != 1 || n < 0);

	recursion(n);

	printf("Stacktrace: %zu%s semi-valid stack frames found\n", num_entries, num_entries == 256 ? " (of 256 max)" : "");
	for (size_t i = 0; i < (size_t)num_entries; ++i)
	{
		printf("0x%0*zX\n", sizeof(void*) * 2, (size_t)buffer[i]);
	}
}
