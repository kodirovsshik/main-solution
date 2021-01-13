#include <Windows.h>
#include <TlHelp32.h>

#include <stdio.h>

#include <SFML/Network.hpp>

bool get_debugger_privilegies()
{
	HANDLE current_token;
	LUID luid;
	TOKEN_PRIVILEGES priveleges;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &current_token);

	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

	priveleges.PrivilegeCount = 1;
	priveleges.Privileges[0].Luid = luid;
	priveleges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	bool result = AdjustTokenPrivileges(current_token, false, &priveleges, sizeof(priveleges), NULL, NULL);

	CloseHandle(current_token);
	return result;
}

[[noreturn]] void critical(int code = 1, const char* fmt = "", ...)
{
	va_list ap;
	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);

	va_end(ap);
	
	fwrite("\a\nFatal error, press Enter to exit", sizeof(char), 34, stderr);
	rewind(stdin);
	while (getchar() != '\n');

	exit(code);
}
[[noreturn]] void wcritical(int code = 1, const wchar_t* fmt = L"", ...)
{
	va_list ap;
	va_start(ap, fmt);

	vfwprintf(stderr, fmt, ap);

	va_end(ap);

	fwrite("\a\nFatal error, press Enter to exit", sizeof(char), 34, stderr);
	rewind(stdin);
	while (getchar() != '\n');

	exit(code);
}

DWORD get_pid_by_name(const wchar_t* exe_name)
{
	HANDLE snapshot;
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(entry);
	
	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32FirstW(snapshot, &entry) == false)
	{
		return -1;
	}

	do 
	{
		if (_wcsicmp(exe_name, entry.szExeFile) == 0)
		{
			return entry.th32ProcessID;
		}
	} while (Process32NextW(snapshot, &entry));

	return -1;
}

DWORD WINAPI worker(LPVOID __parameter)
{
	while (1) {};

	void** args = (void**)__parameter;

	
	int x = 0;
	x /= 0;
	return 0;

	__asm nop
	__asm nop
	__asm nop
	__asm nop
	__asm nop
	__asm nop
	__asm nop
	__asm nop
}

void* memfind(const void* p_block, size_t length, const void* p_pattern, size_t pattern_length)
{
	const uint8_t* p_last = (const uint8_t*)p_block + length - pattern_length;
	const uint8_t* p = (const uint8_t*)p_block;
	while (p != p_last)
	{
		if (memcmp(p, p_pattern, pattern_length) == 0)
			return (void*)p;
		p++;
	}
	return NULL;
}

const void* extract_actual_function_address(const void* p_jump)
{
	uint8_t jump_opcode = *(uint8_t*)p_jump;
	int32_t offset = *( (uint32_t*)((uint8_t*)p_jump + 1) );

	if (jump_opcode != 0xE9)
	{
		return NULL;
	}

	return (const void*)((uint8_t*)p_jump + offset + 5);
}

int main()
{
	
	//worker(NULL);
	if (!get_debugger_privilegies())
	{
		critical(1, "Failed to retrieve debugger privileges");
	}
	else
	{
		printf("Debug privileges granted\n");
	}

	const wchar_t* proc_name = L"test1.exe";

	wprintf(L"%s \"%s\"\n", L"Looking for process", proc_name);
	DWORD proc_id = get_pid_by_name(proc_name);
	if (proc_id == -1)
	{
		wcritical(2, L"Failed to find process %s", proc_name);
	}

	printf("%s", "Trying to open the process\n");
	HANDLE proc_handle = OpenProcess(PROCESS_ALL_ACCESS, false, proc_id);
	if (proc_handle == NULL)
	{
		wcritical(3, L"Failed to find process %s", proc_name);
	}

	printf("Calculating injectee size: ");
	size_t func_size;

	{
		uint64_t temp = 0x9090909090909090;
		const void* func_addr = extract_actual_function_address(&worker);
		uint8_t* func_end = (uint8_t*)memfind(func_addr, -1, &temp, 8);

		if (func_end == NULL)
		{
			critical(4, "Failed to calculate injected function's size\n");
		}

		func_size = func_end - (uint8_t*)func_addr;
		printf("%zu\n", func_size);
	}

	printf("Allocating the memory to inject the code\n");
	LPVOID p_memory = VirtualAllocEx(
		proc_handle,
		NULL,
		func_size,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE
	);

	if (p_memory == NULL)
	{
		wcritical(5, L"Failed to allocate memory inside %s", proc_name);
	}

	ULONG written;
	if (WriteProcessMemory(proc_handle, p_memory, &worker, func_size, &written) == FALSE)
	{
		critical(6, "Failed to inject code");
	}

	if (written == func_size)
	{
		printf("The code was successfully injected\n");
	}
	else
	{
		critical(7, "Failed to inject the code");
	}
	
	printf("Creating a remote thread\n");
	auto attach_result = CreateRemoteThread(
		proc_handle,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)p_memory,
		NULL,
		0,//CREATE_SUSPENDED,
		NULL
		);

	if (attach_result == NULL)
	{
		critical(8, "Failed to create a remote thread");
	}

	printf("Everything done, waiting for the process to finish\n");

	DWORD exit_code;
	if (GetExitCodeProcess(proc_handle, &exit_code) == FALSE)
	{
		critical(8, "Failed to start waiting for a process termination");
	}
	printf("Process has exited with return code %zu\n", size_t(exit_code));

	CloseHandle(proc_handle);
	CloseHandle(attach_result);

	return 0;
}