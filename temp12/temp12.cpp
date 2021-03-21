
#define CL_TARGET_OPENCL_VERSION 110
#define CL_MINIMUM_OPENCL_VERSION 110

#pragma comment(lib, "OpenCL.lib")
#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "runtimeobject.lib")

#pragma comment(lib, "libksn_opencl_selector.lib")

#pragma warning(disable : 4996)

#include <ksn/opencl_selector.hpp>
#include <ksn/stuff.hpp>

#include <string.h>
#include <immintrin.h>

const char* const srcs[] = 
{
	R"(

__kernel void worker(__global float* p)
{
	float x = 1;
	//Performs 1024 fp op

	for (int i = 0; i < 4; ++i)
	{
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
		x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f; x = x * x * x * 2 + 1.3f;
	}
	*p = x;
}

)"
};

int main()
{
	int temp = 0;

	size_t _len = strlen(srcs[0]);

	ksn::opencl_selector_data_t data;
	//data.cl_build_parameters = "-cl-opt-disable";
	data.cl_sources = srcs;
	data.cl_sources_lengthes = &_len;
	data.cl_sources_number = 1;
	data.build_log_file_name = L"OpenCL.log";
	//data.platform = 1;
	//data.device = 1;
	data.opencl_minor = 1;
	temp = ksn::opencl_selector(&data);
	if (temp) return temp;

	cl_device_id device;
	temp = clGetCommandQueueInfo(data.q, CL_QUEUE_DEVICE, sizeof(device), &device, nullptr);

	{
		cl_platform_id id;
		clGetDeviceInfo(device, CL_DEVICE_PLATFORM, sizeof(id), &id, nullptr);
		size_t len;
		clGetPlatformInfo(id, CL_PLATFORM_NAME, 0, NULL, &len);
		char* buffer = (char*)malloc(len);
		if (buffer == nullptr) return -1;
		clGetPlatformInfo(id, CL_PLATFORM_NAME, len, buffer, nullptr);
		printf("%s\n", buffer);
		free(buffer);
	}
	{
		size_t len;
		clGetDeviceInfo(device, CL_DEVICE_VENDOR, 0, NULL, &len);
		char* buffer = (char*)malloc(len);
		if (buffer == nullptr) return -1;
		clGetDeviceInfo(device, CL_DEVICE_VENDOR, len, buffer, nullptr);
		printf("%s\n", buffer);
		free(buffer);
	}

	cl_kernel worker = clCreateKernel(data.program, "worker", nullptr);

	size_t local_size = 0;
	temp = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(local_size), &local_size, nullptr);

	size_t global_offset = 0;
	size_t op_per_call = 1024;

	size_t work_groups = 0;
	printf("Work groups count: ");
	(void)scanf("%zu", &work_groups);

	cl_mem buff = clCreateBuffer(data.context, 0, 4, nullptr, &temp);
	temp = clSetKernelArg(worker, 0, sizeof(buff), &buff);

	size_t calls = work_groups * local_size;

	float result;
	uint64_t delta_t = ksn::measure_running_time_no_return([&]()
	{

		temp = clEnqueueNDRangeKernel(data.q, worker, 1, &global_offset, &calls, &local_size, 0, nullptr, nullptr);
		temp = clEnqueueReadBuffer(data.q, buff, CL_TRUE, 0, 4, &result, 0, 0, 0);
		temp = clFinish(data.q);
	});

	size_t total_op = op_per_call * calls;
	float gflops = float(total_op) / delta_t;

	printf("%g GFLOPS\n%g bil fp operations in %g ms", gflops, float(total_op) / 1024 / 1024 / 1024, float(delta_t) / 1000 / 1000);
}
