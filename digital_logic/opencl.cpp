

#include "opencl.hpp"
#include "err_handling.hpp"
#include "window.hpp"

#include <ksn/stuff.hpp>

#include <string>
#include <execution>



#pragma warning(disable : 4996)





constexpr static std::string_view cl_sources[] =
{
R"(

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]

typedef int int32_t;
typedef uint uint32_t;
typedef short int16_t;
typedef ushort uint16_t;
typedef uchar uint8_t;

__kernel void kernel_downscale(const __global uchar4* in, __global uchar4* out, uint16_t out_width, uint16_t out_height, uint8_t factor)
{
	size_t id = get_global_id(0);
	size_t x = id % out_width;
	size_t y = id / out_width;
	uint16_t in_width = out_width * factor;

	ushort4 result = 0;
	out += id;
	in += factor * (x + y * in_width); //This formula worked fine on paper

	size_t in_jump_size = in_width - factor;


	for (uint8_t i = factor; i --> 0; )
	{
		for (uint8_t j = factor; j --> 0; )
			result += convert_ushort4(*in++);

		in += in_jump_size;
	}

	factor *= factor;
	*out = convert_uchar4(result / factor); //Mean value math guarantees that all of them are in [0; 255]
}



struct sprite_data_t
{
	ushort2 m_texture_size;
	ushort2 m_sprite_size;
	ushort2 m_sprite_texture_offset;
};


struct transform_data_t
{
	float2 m_position;
	float2 m_position_origin;
	float2  m_rotation_origin;
	float2 m_rotation_data;
};

STATIC_ASSERT(sizeof(struct sprite_data_t) == 12, SIZE_SP_12);
STATIC_ASSERT(sizeof(struct transform_data_t) == 32, SIZE_TXT_32);

void draw_pixel(__global const uchar4* p_sprite_data, __global uchar4* const p_screen_data, float2 screen_pos, ushort2 window_size)
{
	if (screen_pos.x < 0 || screen_pos.y < 0 || screen_pos.x >= window_size.x || screen_pos.y >= window_size.y)
		return;

	__global uchar4* p_screen = p_screen_data + (int)screen_pos.y * window_size.x + (int)screen_pos.x;

	float t = (*p_sprite_data).w / 255.f;

	uchar4 color = *p_screen + convert_uchar4((convert_float4(*p_sprite_data) - convert_float4(*p_screen_data)) * t);

	*p_screen = color;
}

__kernel void kernel_draw_sprite_default(__global const uchar4* p_sprite_data, __global uchar4* const p_screen_data, struct transform_data_t tdata, struct sprite_data_t spdata, ushort2 window_size, uint8_t upscale_factor)
{
	size_t id = get_global_id(0);
	if (id >= spdata.m_sprite_size.x * spdata.m_sprite_size.y) return;

	ushort2 work_item_pos = (ushort2)( id % spdata.m_sprite_size.x, id / spdata.m_sprite_size.x );

	ushort2 sprite_pos = work_item_pos + spdata.m_sprite_texture_offset;
	p_sprite_data += sprite_pos.y * spdata.m_texture_size.x + sprite_pos.x;

	window_size *= upscale_factor;

	for (uint8_t i = upscale_factor; i-- > 0;)
	{
		for (uint8_t j = upscale_factor; j-- > 0;)
		{
			float2 screen_pos = upscale_factor * (convert_float2(work_item_pos) - tdata.m_rotation_origin) + (float2)( j, i );
			screen_pos = (float2)( screen_pos.x * tdata.m_rotation_data.y - screen_pos.y * tdata.m_rotation_data.x, screen_pos.x * tdata.m_rotation_data.x+ screen_pos.y * tdata.m_rotation_data.y );
			screen_pos += upscale_factor * (tdata.m_rotation_origin + tdata.m_position - tdata.m_position_origin);

			const float tolerance = 0.1;
			float2 round_delta = fabs(screen_pos - round(screen_pos));
			if (round_delta.x > tolerance)
			{
				if (round_delta.y > tolerance)
				{
					draw_pixel(p_sprite_data, p_screen_data, floor(screen_pos) + (float2)(0, 0), window_size);
					draw_pixel(p_sprite_data, p_screen_data, floor(screen_pos) + (float2)(0, 1), window_size);
					draw_pixel(p_sprite_data, p_screen_data, floor(screen_pos) + (float2)(1, 0), window_size);
					draw_pixel(p_sprite_data, p_screen_data, floor(screen_pos) + (float2)(1, 1), window_size);
				}
				else
				{
					screen_pos.y = round(screen_pos.y);
					draw_pixel(p_sprite_data, p_screen_data, floor(screen_pos) + (float2)(0, 0), window_size);
					draw_pixel(p_sprite_data, p_screen_data, floor(screen_pos) + (float2)(1, 0), window_size);
				}
			}
			else
			{
				if (round_delta.y > tolerance)
				{
					screen_pos.x = round(screen_pos.x);
					draw_pixel(p_sprite_data, p_screen_data, floor(screen_pos) + (float2)(0, 0), window_size);
					draw_pixel(p_sprite_data, p_screen_data, floor(screen_pos) + (float2)(0, 1), window_size);
				}
				else
				{
					draw_pixel(p_sprite_data, p_screen_data, round(screen_pos) + (float2)(0, 0), window_size);
				}
			}
		}
	}
}

__kernel void kernel_clear(__global uchar4* out, uchar4 color, uint16_t win_width, uint8_t factor)
{
	size_t id = get_global_id(0);
	uint16_t x = id % win_width;
	uint16_t y = id / win_width;

	out += (y * win_width * factor + x) * factor;
	size_t out_jump_size = factor * (win_width - 1);

	for (uint8_t i = factor; i --> 0; )
	{
		for (uint8_t j = factor; j --> 0; )
			 *out++ = color;

		out += out_jump_size;
	}
}
)"
};

static constexpr size_t cl_sources_lengthes[] =
{
	cl_sources[0].length(),
};


static_assert(ksn::countof(cl_sources) == ksn::countof(cl_sources_lengthes));





void postinit_opencl()
{
	cl_data.device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &cl_data.max_work_group_size);
	
	cl_uint dims;
	cl_data.device.getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &dims);

	std::vector<size_t> work_sizes(dims);
	cl_data.device.getInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, &work_sizes);
	cl_data.max_work_items = work_sizes[0];

	cl_ulong max_alloc;
	cl_data.device.getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, &max_alloc);
	cl_data.max_alloc_size = (size_t)max_alloc;
	
	cl_data.kernel_downscale = cl::Kernel(cl_data.program, "kernel_downscale");
	cl_data.kernel_draw_sprite_default = cl::Kernel(cl_data.program, "kernel_draw_sprite_default");
	cl_data.kernel_clear = cl::Kernel(cl_data.program, "kernel_clear");
}


static void filter_devices(std::vector<cl::Device>& devices)
{
	devices.resize(std::remove_if(std::execution::par_unseq, devices.begin(), devices.end(), []
	(const cl::Device& d)
		{
			cl_bool tbool;

			d.getInfo(CL_DEVICE_AVAILABLE, &tbool);
			if (tbool == false) return true;

			d.getInfo(CL_DEVICE_COMPILER_AVAILABLE, &tbool);
			if (tbool == false) return true;

			std::string tstr;
			d.getInfo(CL_DEVICE_VERSION, &tstr);

			int v1 = 0, v2 = 0;
			(void)sscanf(tstr.c_str() + 7, "%i.%i", &v1, &v2);

			if (v1 < DIGILOG_CL_VERSION_MAJOR || (v1 == DIGILOG_CL_VERSION_MAJOR && v2 < DIGILOG_CL_VERSION_MINOR))
				return true;

			return false;
		}
	) - devices.begin());
}

void _init_opencl_default()
{
	cl_int err;

	//cl_data.context = cl::Context(CL_DEVICE_TYPE_ALL);

	std::vector<cl::Device> devices;
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	for (const auto& platform : platforms)
	{
		std::vector<cl::Device> local_devices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &local_devices);

		devices.reserve(devices.size() + local_devices.size());
		std::copy(local_devices.begin(), local_devices.end(), std::back_inserter(devices));
	}

	critical_assert1(devices.size() != 0, -1, "Critical error", "No OpenCL devices found in the system");

	//Remove every device which is:
	//Not available
	//Or have no compiler
	//Or has obsolete OpenCL support
	filter_devices(devices);

	critical_assert1(devices.size() != 0, -1, "Critical error", "No situable OpenCL devices found in the system");

	//Try to find the best device is such priority:
	//Type:
	//	GPU
	//	ACCELERATOR
	//	CPU
	//	CUSTOM = 1 << 4
	//	DEFAULT
	//
	//Profile:
	//	Full
	//	Embeded
	//
	//Driver version:
	//	(the higher the better)
	cl_data.device = *std::max_element(std::execution::par_unseq, devices.begin(), devices.end(), []
	(const cl::Device& a, const cl::Device& b) -> bool
		{
			std::string tstr;
			cl_device_type dtype1;
			cl_device_type dtype2;

			a.getInfo(CL_DEVICE_TYPE, &dtype1);
			b.getInfo(CL_DEVICE_TYPE, &dtype2);

			size_t priority1 = 0;
			size_t priority2 = 0;

			auto update_priority = []
			(size_t& priority, cl_device_type& dtype)
			{
				if (dtype & CL_DEVICE_TYPE_DEFAULT) priority |= 1;
				if (dtype & (1 << 4)) priority |= 2;
				if (dtype & CL_DEVICE_TYPE_CPU) priority |= 4;
				if (dtype & CL_DEVICE_TYPE_ACCELERATOR) priority |= 8;
				if (dtype & CL_DEVICE_TYPE_GPU) priority |= 16;
			};

			update_priority(priority1, dtype1);
			update_priority(priority2, dtype2);

			if (priority1 != priority2)
				return priority1 < priority2;


			cl_bool fprofile1 = false;
			cl_bool fprofile2 = false;

			a.getInfo(CL_DEVICE_PROFILE, &fprofile1);
			b.getInfo(CL_DEVICE_PROFILE, &fprofile2);

			if (fprofile1 != fprofile2)
				return !fprofile1 && fprofile2;


			a.getInfo(CL_DRIVER_VERSION, &tstr);
			int va1 = 0, va2 = 0;
			(void)sscanf(tstr.c_str() + 7, "%i.%i", &va1, &va2);

			b.getInfo(CL_DRIVER_VERSION, &tstr);
			int vb1 = 0, vb2 = 0;
			(void)sscanf(tstr.c_str() + 7, "%i.%i", &vb1, &vb2);

			if (va1 < vb1) return true;
			if (va1 > vb1) return false;
			if (va2 < vb2) return true;
			if (va2 > vb2) return false;

			return false; //equal
		}
	);

	cl_data.context = cl::Context(cl_data.device);

	cl_data.device.getInfo(CL_DEVICE_PLATFORM, &cl_data.platform);

	cl_data.q = cl::CommandQueue(cl_data.context, cl_data.device);

	
	cl_data.program = cl::Program(
		clCreateProgramWithSource(cl_data.context(), (cl_uint)ksn::countof(cl_sources), (const char**)cl_sources, cl_sources_lengthes, &err), 
		true);
	cl::detail::errHandler(err, "Failed to create cl_program"); //will throw if not ok

	cl_data.program.build(CL_BUILD_PARAMS);
}

void _init_opencl_custom()
{

	auto temp_context = cl::Context(CL_DEVICE_TYPE_ALL);

	std::vector<cl::Device> devices;
	temp_context.getInfo(CL_CONTEXT_DEVICES, &devices);
	critical_assert1(devices.size() != 0, -1, "Fatal error", "No OpenCL devices found in the system");

	filter_devices(devices);
	critical_assert1(devices.size() != 0, -1, "Fatal error", "No situable OpenCL devices found in the system");

	printf("List of situable OpenCL devices found in the system:\n\n");

	cl_int err;
	std::string temps;
	cl::Platform temp_platform;
	for (const auto& device : devices)
	{
		device.getInfo(CL_DEVICE_NAME, &temps);
		printf("[%zi] \"%s\" by \"", &device - &devices[0], temps.c_str());

		device.getInfo(CL_DEVICE_VENDOR, &temps);
		printf("%s\" on \"", temps.c_str());

		device.getInfo(CL_DEVICE_PLATFORM, &temp_platform);
		temp_platform.getInfo(CL_PLATFORM_NAME, &temps);
		printf("%s\"\n", temps.c_str());
	}

	putc('\n', stdout);

	int select;
	while (true)
	{
		printf("Select a device: ");

		int result = scanf("%i", &select);
		if (result != 1 || select < 0 || select >= devices.size())
			rewind(stdin);
		else
			break;
	}


	cl_data.context = cl::Context(cl_data.device = devices[select]);
	
	cl_data.device.getInfo(CL_DEVICE_PLATFORM, &cl_data.platform);

	cl_data.program = cl::Program(clCreateProgramWithSource
		(cl_data.context.get(), (cl_uint)ksn::countof(cl_sources), (const char**)cl_sources, cl_sources_lengthes, &err)
		, true);

	cl::detail::errHandler(err);

	cl_data.program.build(CL_BUILD_PARAMS);

	cl_data.q = cl::CommandQueue(cl_data.context, cl_data.device);
}

void _init_opencl_opengl_interop()
{
	cl_context_properties properties[] =
	{
		CL_GL_CONTEXT_KHR, 0, (cl_context_properties)window.window.context_native_handle(),
		CL_WGL_HDC_KHR, 0, (cl_context_properties)window.window.winapi_get_hdc(),
	};

	cl_data.context = cl::Context(CL_DEVICE_TYPE_ALL, properties);

	if constexpr (true)
	{
		std::vector<cl::Device> devices;
		cl_data.context.getInfo(CL_CONTEXT_DEVICES, &devices);

		critical_assert1(devices.size() == 1, -1, "", "Failed to create an OpenCL device from an OpenGL context");

		cl_data.device = std::move(devices.front());
	}

	cl_int err;

	cl_data.program = cl::Program(
		clCreateProgramWithSource(cl_data.context.get(), (cl_uint)ksn::countof(cl_sources), (const char**)cl_sources, cl_sources_lengthes, &err),
		true);

	cl::detail::errHandler(err, "Failed to create cl_program"); //will throw if not ok

	cl_data.program.build(CL_BUILD_PARAMS);

	cl_data.q = cl::CommandQueue(cl_data.context, cl_data.device);
}

void init_opencl()
{
	_init_opencl_opengl_interop();

	postinit_opencl();
}
