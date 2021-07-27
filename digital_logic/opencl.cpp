

#include "opencl.hpp"
#include "err_handling.hpp"
#include "window.hpp"

#include <ksn/stuff.hpp>

#include <string>
#include <execution>



#pragma warning(disable : 4996)




const static std::string cl_src =
{
R"(

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]

typedef int int32_t;
typedef uint uint32_t;
typedef short int16_t;
typedef ushort uint16_t;
typedef uchar uint8_t;

__kernel void kernel_downscale(const __global uchar* in, __global uchar* out, uint16_t out_width, uint16_t out_height, uint8_t factor)
{
	size_t id = get_global_id(0);
	size_t x = id % out_width;
	size_t y = id / out_width;
	uint16_t in_width = out_width * factor;

	ushort3 result = 0;
	out += id;
	in += factor * (x + y * in_width) * 3;

	size_t in_jump_size = in_width - factor;
	//in_jump_size *= 3;
	in_jump_size = in_jump_size * 2 + in_jump_size;

	for (uint8_t i = factor; i --> 0; )
	{
		for (uint8_t j = factor; j --> 0; )
		{
			result += (ushort3)(in[0], in[1], in[2]);
			in += 3;
		}

		in += in_jump_size;
	}

	factor *= factor;
	result /= factor; //Mean value math guarantees that all of the components are in [0; 255]
	//return;
	*out++ = result.x;
	*out++ = result.y;
	*out = result.z;
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

void draw_pixel(__constant const uchar4* p_sprite_data, __global uchar* const p_screen_data, float2 screen_pos, ushort2 window_size)
{
	if (screen_pos.x < 0 || screen_pos.y < 0 || screen_pos.x >= window_size.x || screen_pos.y >= window_size.y)
		return;

	__global uchar* p_screen = p_screen_data + ((int)screen_pos.y * window_size.x + (int)screen_pos.x) * 3;

	float t = (*p_sprite_data).w / 255.f;

	float4 dcolor = convert_float4(*p_sprite_data) - convert_float4(*p_screen_data);
	dcolor *= t;
	uchar3 color = (uchar3)(p_screen[0], p_screen[1], p_screen[2]);
	color += convert_uchar4(dcolor).xyz;

	//return;
	*p_screen++ = color.x;
	*p_screen++ = color.y;
	*p_screen = color.z;
}

__kernel void kernel_draw_sprite_default(__constant const uchar4* p_sprite_data, __global uchar* const p_screen_data, struct transform_data_t tdata, struct sprite_data_t spdata, ushort2 window_size, uint8_t upscale_factor)
{
	size_t id = get_global_id(0);
	if (id >= spdata.m_sprite_size.x * spdata.m_sprite_size.y) return;

	ushort2 work_item_pos = (ushort2)( id % spdata.m_sprite_size.x, id / spdata.m_sprite_size.x );

	ushort2 sprite_pos = work_item_pos + spdata.m_sprite_texture_offset;
	p_sprite_data += sprite_pos.y * spdata.m_texture_size.x + sprite_pos.x;

	window_size *= upscale_factor;

	for (uint8_t i = upscale_factor; i --> 0;)
	{
		for (uint8_t j = upscale_factor; j --> 0;)
		{
			float2 screen_pos = upscale_factor * (convert_float2(work_item_pos) - tdata.m_rotation_origin) + (float2)( j, i );
			screen_pos = (float2)(
				screen_pos.x * tdata.m_rotation_data.y - screen_pos.y * tdata.m_rotation_data.x,
				screen_pos.x * tdata.m_rotation_data.x + screen_pos.y * tdata.m_rotation_data.y
			);
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

__kernel void kernel_clear(__global uchar* out, uchar4 color, uint16_t win_width, uint8_t factor)
{
	size_t id = get_global_id(0);
	uint16_t x = id % win_width;
	uint16_t y = id / win_width;

	out += (y * win_width * factor + x) * factor * 3;
	size_t out_jump_size = factor * (win_width - 1);

	for (uint8_t i = factor; i --> 0; )
	{
		for (uint8_t j = factor; j --> 0; )
		{
			 *out++ = color.x;
			 *out++ = color.y;
			 *out++ = color.z;
		}
			//color = color;

		out += out_jump_size;
	}
}

)" 

#if DIGILOG_USE_OPENGL

R"(

__kernel void kernel_to_gl_renderbuffer(__global __read_only const uchar* in, __write_only image2d_t out, ushort2 size)
{
	size_t id = get_global_id(0);
	int2 pos = (int2)(id % size.x, size.y - 1 - id / size.x);

	in += id * 3;
	write_imagef(out, pos, (float4)(in[2] / 255.f, in[1] / 255.f, in[0] / 255.f, 0.f));
}

)"

#endif
};








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
#if DIGILOG_USE_OPENGL
	cl_data.kernel_to_gl_renderbuffer = cl::Kernel(cl_data.program, "kernel_to_gl_renderbuffer");
#endif
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

			int v1 = -1, v2 = -1;
			(void)sscanf(tstr.c_str() + 7, "%i.%i", &v1, &v2);

			if (v1 < DIGILOG_CL_VERSION_MAJOR || (v1 == DIGILOG_CL_VERSION_MAJOR && v2 < DIGILOG_CL_VERSION_MINOR))
				return true;

			return false;
		}
	) - devices.begin());
}



#if DIGILOG_USE_OPENGL

void _init_opencl_opengl_interop()
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	std::string ext_string;

	bool ok_context = false;

	for (auto& current_platform : platforms)
	{
		current_platform.getInfo(CL_PLATFORM_EXTENSIONS, &ext_string);
		if (ext_string.find("cl_khr_gl_sharing") == std::string::npos)
			continue;

		cl_context_properties properties[] =
		{
			CL_GL_CONTEXT_KHR, (cl_context_properties)window.window.context_native_handle(),
			CL_WGL_HDC_KHR, (cl_context_properties)window.window.winapi_get_hdc(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)current_platform(),
			0
		};

		std::vector<cl::Device> devices;
		current_platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
		filter_devices(devices);

		for (auto& current_device : devices)
		{
			current_device.getInfo(CL_DEVICE_EXTENSIONS, &ext_string);
			if (ext_string.find("cl_khr_gl_sharing") == std::string::npos)
				continue;

			try
			{
				cl_data.context = cl::Context(current_device, properties);
			}
			catch (const cl::Error& err)
			{
				if constexpr (_KSN_IS_DEBUG_BUILD)
				{
					fprintf(stderr, "Context creation failure: %i\n", err.err());
				}
				continue;
			}

			ok_context = true;
			cl_data.device = std::move(current_device);
			break;
		}

		if (!ok_context) continue;

		cl_data.platform = std::move(current_platform);
		break;
	}

	if (!ok_context)
	{
		throw std::exception("Faied to create an OpenGL-shared OpenCL context");
	}

	cl_data.program = cl::Program(cl_data.context, cl_src, true);

	cl_data.q = cl::CommandQueue(cl_data.context, cl_data.device);
}

void init_opencl()
{
	_init_opencl_opengl_interop();

	postinit_opencl();
}

#else

void _init_opencl_default()
{
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
			cl_device_type dtype1;
			cl_device_type dtype2;

			a.getInfo(CL_DEVICE_TYPE, &dtype1);
			b.getInfo(CL_DEVICE_TYPE, &dtype2);

			size_t priority1 = 0;
			size_t priority2 = 0;

			auto update_priority = []
			(size_t& priority, cl_device_type dtype)
			{
				if (dtype & CL_DEVICE_TYPE_GPU) priority |= 16;
				if (dtype & CL_DEVICE_TYPE_ACCELERATOR) priority |= 8;
				if (dtype & CL_DEVICE_TYPE_CPU) priority |= 4;
				if (dtype & (1 << 4)) priority |= 2;
				if (dtype & CL_DEVICE_TYPE_DEFAULT) priority |= 1;
			};

			update_priority(priority1, dtype1);
			update_priority(priority2, dtype2);

			if (priority1 != priority2)
				return priority1 < priority2;


			cl_bool full_profile1 = false;
			cl_bool full_profile2 = false;

			a.getInfo(CL_DEVICE_PROFILE, &full_profile1);
			b.getInfo(CL_DEVICE_PROFILE, &full_profile2);

			if (full_profile1 != full_profile2)
				return !full_profile1 && full_profile2;


			std::string temp_str;
			a.getInfo(CL_DRIVER_VERSION, &temp_str);
			int version1_major = 0, version1_minor = 0;
			(void)sscanf(temp_str.c_str() + 7, "%i.%i", &version1_major, &version1_minor);

			b.getInfo(CL_DRIVER_VERSION, &temp_str);
			int version2_major = 0, version2_minor = 0;
			(void)sscanf(temp_str.c_str() + 7, "%i.%i", &version2_major, &version2_minor);

			if (version1_major < version2_major) return true;
			if (version1_major > version2_major) return false;
			if (version1_minor < version2_minor) return true;
			if (version1_minor > version2_minor) return false;

			return false; //equal
		}
	);

	cl_data.context = cl::Context(cl_data.device);

	cl_data.device.getInfo(CL_DEVICE_PLATFORM, &cl_data.platform);

	cl_data.program = cl::Program(cl_data.context, cl_src, true);

	cl_data.q = cl::CommandQueue(cl_data.context, cl_data.device);
#if !DIGILOG_USE_OPENGL
	cl_data.q2 = cl::CommandQueue(cl_data.context, cl_data.device);
#endif
}


void init_opencl()
{
	_init_opencl_default();

	postinit_opencl();
}

#endif
