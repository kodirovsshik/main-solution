
#define CL_HPP_ENABLE_EXCEPTIONS 1
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120



#include <CL/opencl.hpp>

#include <stdint.h>
#include <stdio.h>

#include <Windows.h>

#include <ksn/stuff.hpp>


#pragma comment(lib, "OpenCL.lib")
#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "runtimeobject.lib")



#ifdef _DEBUG
#pragma comment(lib, "libksn_stuff-s-d.lib")
#pragma comment(lib, "libksn_x86_instruction_set-s-d")
#else
#pragma comment(lib, "libksn_stuff-s.lib")
#pragma comment(lib, "libksn_x86_instruction_set-s")
#endif


#pragma warning(disable : 4996)





[[noreturn]] void блять(const char* msg = "")
{
	MessageBoxA(GetConsoleWindow(), msg, "блять", MB_ICONERROR);
	abort();
}
[[noreturn]] void блять1(const char* fmt = "", ...)
{
	char buffer[4096];

	va_list ap;
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	va_end(ap);

	блять(buffer);
}


extern const char* test_prog; 


void foo(std::vector<int>& v)
{
	for (int& elem : v)
	{
		elem = elem * 2 + 1;
	}
}

int main()
{
	bool ok = false;

	try
	{
		size_t temp = 0;
		cl::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		ksn_dynamic_assert(platforms.size() > 0, "No OpenCL implementations found in the system");

		auto platform = platforms.front();

		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

		ksn_dynamic_assert(devices.size() > 0, "No OpenCL-compatible devices found on main platform");

		printf("Main platform devices:\n\n");
		for (const auto& device : devices)
		{
			auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
			auto version = device.getInfo<CL_DEVICE_VERSION>();
			printf("[%zu]: %s by %s\n\n", temp++, version.c_str(), vendor.c_str());
		}

		printf("%zu device%s total\nSelecting [0] as default\n\n", devices.size(), devices.size() == 1 ? "" : "s");


		cl::Program::Sources sources{ test_prog };
		cl::Context context(devices);
		cl::Program prog(context, sources);
		prog.build("-cl-std=CL1.2");
		cl::Kernel kernel_iota(prog, "iota");
		
		constexpr size_t N = 1 << 24;
		std::vector<int> arr(N, 2);
		foo(arr);
		foo(arr);

		cl::Buffer buff(context, CL_MEM_USE_HOST_PTR, sizeof(int) * N, arr.data());
		

		cl::CommandQueue q(context);
		q.enqueueWriteBuffer(buff, CL_TRUE, 0, sizeof(int) * N, arr.data());
		q.enqueueNDRangeKernel(kernel, 0, N);
		q.enqueueNDRangeKernel(kernel, 0, N);
		q.enqueueReadBuffer(buff, CL_TRUE, 0, sizeof(int) * N, arr.data());



		ok = true;
	}
	catch (const cl::BuildError& error)
	{
		auto log = error.getBuildLog();
		for (const auto& entry : log)
		{
			printf("%s\n", entry.second.c_str());
		}
	}
	catch (const cl::Error& error)
	{
		printf("OpenCL error %i: %s\n", error.err(), error.what());
	}
	catch (const std::exception& excp)
	{
		printf("Unexpected error caught: %s", excp.what());
	}

	if (!ok)
	{
		ksn_dynamic_assert(false, "");
	}


	printf("\n\nDone.\n");
	(void)getchar();
	return 0;
}