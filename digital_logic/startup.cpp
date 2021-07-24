
#include <exception>
#include <filesystem>

#include <stdio.h>

#include <ksn/stuff.hpp>

#define NOMINMAX
#include <Windows.h>

#include "err_handling.hpp"

#include "opencl.hpp"




void process_exception(const std::exception& excp, int level = 0)
{
	fprintf(stderr, "\n\
UNHANDLED %s EXCEPTION: std::exception\n\
what() = %s\n\
", level == 0 ? "" : "NESTED", excp.what());
	critical(-1, "", "");
	try
	{
		std::rethrow_if_nested(excp);
	}
	catch (const std::exception& excp)
	{
		process_exception(excp, level + 1);
	}
}


int main(int argc, char** argv)
{
	EXCEPTION_POINTERS* excp_info = nullptr;

	__try
	{
		int wrapper(int, char**);
		return wrapper(argc, argv);
	}
	__except ((excp_info = GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		fprintf(stderr, "\n\
UNHANDLED SEH EXCEPTION:");

		if (excp_info)
		{
			//fprintf(stderr, "\n\nContext record memory dump:\n");
			//ksn::memory_dump(excp_info->ContextRecord, sizeof(*excp_info->ContextRecord), -1, ksn::memory_dump.no_space);

			//fprintf(stderr, "\n\nException record memory dump:\n");
			//ksn::memory_dump(excp_info->ExceptionRecord, sizeof(*excp_info->ExceptionRecord), -1, ksn::memory_dump.no_space);

			fprintf(stderr, "\n\n");

			return (int)excp_info->ExceptionRecord->ExceptionCode;
		}
		else
		{
			fprintf(stderr, "\nGetExceptionInformation() is NULL\n");
			return -1;
		}
	}
}


int wrapper(int argc, char** argv)
{
	try
	{
		int wrapped_main(int, char**);
		return wrapped_main(argc, argv);
	}


	catch (const std::filesystem::filesystem_error& fs)
	{
		fprintf(stderr, "\n\
UNHANDLED EXCEPTION: std::filesystem::filesystem_error\n\
std::filesystem::filesystem_error data: \n\
Path1 = \"%s\"\n\
Path2 = \"%s\"\n\
what() = %s\n\
\n\
std::error_code data:\n\
message() = %s\n\
value() = %i\n\
category().name() = %s\
", fs.path1().string().c_str(), fs.path2().string().c_str(), fs.what(), fs.code().message().c_str(), fs.code().value(), fs.code().category().name());
		critical(fs.code().value(), "", "");
	}


	catch (const std::bad_alloc& excp)
	{
		fprintf(stderr, "\n\
UNHANDLED EXCEPTION: std::bad_alloc\n\
what() = %s\n\
", excp.what());
		critical(-1, "", "");
	}


	catch (const std::runtime_error& excp)
	{
		fprintf(stderr, "\n\
UNHANDLED EXCEPTION: std::runtime_error\n\
what() = %s\n\
", excp.what());
		critical(-1, "", "");
	}


	catch (const std::logic_error& excp)
	{
		fprintf(stderr, "\n\
UNHANDLED EXCEPTION: std::logic_error\n\
what() = %s\n\
", excp.what());
		critical(-1, "", "");
	}

	
	catch (const cl::BuildError& berr)
	{
		fprintf(stderr, "\n\
UNHANDLED EXCEPTION: cl::BuildError\n\
err() = %i\n\
what() = %s\n\n\
Build log:\n\n", (int)berr.err(), berr.what());

		std::string temp;
		for (const auto& [device, message] : berr.getBuildLog())
		{
			device.getInfo(CL_DEVICE_NAME, &temp);
			fprintf(stderr, "Device \"%s\" by ", temp.c_str());

			device.getInfo(CL_DEVICE_VENDOR, &temp);
			fprintf(stderr, "\"%s\" has reported:\n%s\n\n", temp.c_str(), message.c_str());
		}
		critical((int)berr.err(), "\nBuild log end.", "");
	}


	catch (const cl::Error& clerr)
	{
		fprintf(stderr, "\n\
UNHANDLED EXCEPTION: cl::Error\n\
err() = %i\n\
what() = %s\n\
", (int)clerr.err(), clerr.what());
		critical((int)clerr.err(), "", "");
	}


	catch (const std::exception& excp)
	{
		process_exception(excp);
		critical(-1, "", "");
	}


	catch (...)
	{
		fprintf(stderr, "\nUNHANDLED EXCEPTION: [unknown exception type]\n");
		critical(-1, "", "");
	}
}
