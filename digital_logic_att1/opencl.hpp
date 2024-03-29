
#ifndef _DIGILOG_OPENCL_HPP_
#define _DIGILOG_OPENCL_HPP_



#define DIGILOG_CL_VERSION_MAJOR 1
#define DIGILOG_CL_VERSION_MINOR 1
#define DIGILOG_CL_VERSION (DIGILOG_CL_VERSION_MAJOR * 100 + DIGILOG_CL_VERSION_MINOR * 10)

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION DIGILOG_CL_VERSION
#define CL_MINIMUM_OPENCL_VERSION DIGILOG_CL_VERSION
#define CL_HPP_TARGET_OPENCL_VERSION DIGILOG_CL_VERSION
#define CL_HPP_MINIMUM_OPENCL_VERSION DIGILOG_CL_VERSION

#include <CL/opencl.hpp>

#define CL_BUILD_PARAMS ""



#include "gl_switch.hpp"



void init_opencl();



struct cl_data_t
{
	cl::Context context;
	cl::Platform platform;
	cl::Program program;
	cl::Device device;
	cl::CommandQueue q;

	cl::Kernel kernel_downscale;
	cl::Kernel kernel_draw_sprite_default;
	cl::Kernel kernel_clear;
#if DIGILOG_USE_OPENGL
	cl::Kernel kernel_to_gl_renderbuffer;
#else
	cl::CommandQueue q2;
#endif

	size_t max_work_group_size = 0;
	size_t max_work_items = 0;
	size_t max_alloc_size = 0;
};

extern cl_data_t cl_data;



#endif //!_DIGILOG_OPENCL_HPP_
