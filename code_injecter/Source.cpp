
#include <Windows.h>
#include <TlHelp32.h>

#include <intrin.h>

#include <stdio.h>
#include <stdint.h>



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

DWORD WINAPI worker(LPVOID param)
{

	MessageBoxW(GetConsoleWindow(), L"ඞ", L"", MB_OK);

	__nop();
	return 0;
}

const void* get_actual_function_address(const void* func)
{
	uint8_t* p = (uint8_t*)func;
	if (*p != 0xE9)
		return func;

	intptr_t offset = *(int32_t*)(p + 1);
	return p + 5 + offset;
}

size_t calculate_injectee_size(const void* func)
{
	func = get_actual_function_address(func);

	uint8_t* p = (uint8_t*)func;

	void* pe = memchr(p, 0x90, (size_t)-1);
	pe = memchr(pe, 0xC3, (size_t)-1);

	size_t size = (uint8_t*)pe - p;
	size = ((size - 1) | 3) + 1;
	return size;
}

//void* memfind(const void* p_block, size_t length, const void* p_pattern, size_t pattern_length)
//{
//	const uint8_t* p_last = (const uint8_t*)p_block + length - pattern_length;
//	const uint8_t* p = (const uint8_t*)p_block;
//	while (p < p_last)
//	{
//		if (memcmp(p, p_pattern, pattern_length) == 0)
//			return (void*)p;
//		p++;
//	}
//	return NULL;
//}



#define check(cond, code, fmt, ...) if (!(cond)) { wcritical(code, fmt, __VA_ARGS__); } else []{}()

int main()
{
	//why you crash

	check(get_debugger_privilegies(), 1, L"Failed to retrieve debugger privileges");

	const wchar_t* proc_name = L"temp14.exe";

	DWORD proc_id = get_pid_by_name(proc_name);
	check(proc_id != -1, 2, L"Failed to find process %s", proc_name);

	HANDLE proc_handle = OpenProcess(PROCESS_ALL_ACCESS, false, proc_id);
	check(proc_handle, 3, L"OpenProcess has failed");

	size_t injectee_size = calculate_injectee_size(worker);

	void* injectee_area_ptr = VirtualAllocEx(proc_handle, NULL, injectee_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	check(injectee_area_ptr, 4, L"VirtualAllocEx has failed");

	size_t written = -1;
	check(WriteProcessMemory(proc_handle, injectee_area_ptr, get_actual_function_address(worker), injectee_size, &written), 5, L"WriteProcessMemory has failed");
	
	DWORD remote_thread_id = 0;
	HANDLE remote_thread_handle = 
		CreateRemoteThread(proc_handle, NULL, 8192, (LPTHREAD_START_ROUTINE)injectee_area_ptr, 
			NULL, CREATE_SUSPENDED, &remote_thread_id);
	check(remote_thread_handle, 6, L"CreateRemoteThread has faied");

	ResumeThread(remote_thread_handle);

	VirtualFreeEx(proc_handle, injectee_area_ptr, 0, MEM_RELEASE);

	return 0;
}