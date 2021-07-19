
//C headers
#include <stdio.h>
#include <stdarg.h>

//C++ headers
#include <exception>

//Local headers
#include "globals.hpp"
#include "window.hpp"

//libksn headers
#include <ksn/stuff.hpp>

//MSVC-Windows-specific headers
#define NOMINMAX
#include <Windows.h>
#include <conio.h>



 extern "C" [[noreturn]] void __digilog_abort();



 [[noreturn]] void __critical(int code, const char* file_path, int line, const char* caption = nullptr, const char* fmt = "", ...)
{
	memset(thread_buffer, 0, thread_buffer_size);

	va_list ap;
	va_start(ap, fmt);
	int max = vsnprintf(thread_buffer, thread_buffer_size - 1, fmt, ap);
	va_end(ap);

	const char* file_name = file_path;
	
	{
		const char* p = file_name;
		char ch;
		while ((ch = *p++) != '\0')
		{
			if (ch == '/' || ch == '\\') file_name = p;
		}
	}

	char buffer[512] = { 0 };

	if (caption == nullptr)
	{
		snprintf(buffer, ksn::countof(buffer), "Critical error at %s:%i", file_name, line);
		caption = buffer;
	}
	
	fprintf(stderr, "\a%s\n", caption);
	fprintf(stderr, "%s\n", thread_buffer);
	fprintf(stderr, "%s", "Press Enter to exit");

	MessageBoxA(window.window.window_native_handle(), "Fatal error has occured, program will be terminated. See console for the details", "Fatal error", MB_ICONERROR);
	window.window.close();

	while (true)
	{
		int ch = _getch();
		if (ch == 10 || ch == 13)
			break;
	}

	exit(code);
	std::terminate();
	std::abort();
	__digilog_abort();
}