
#define CL_TARGET_OPENCL_VERSION 100
#define CL_MINIMUM_OPENCL_VERSION 100

#include <ksn/opencl_selector.hpp>

#pragma comment(lib, "libksn_opencl_selector")

#pragma comment(lib, "OpenCL")
#pragma comment(lib, "runtimeobject")
#pragma comment(lib, "Cfgmgr32")

int main()
{

	ksn::opencl_selector_data_t selector;
	selector.opencl_minor = 0;
	ksn::opencl_selector(&selector);
}