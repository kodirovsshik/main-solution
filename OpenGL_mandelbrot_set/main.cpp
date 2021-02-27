
#define CL_HPP_TARGET_OPENCL_VERSION 110
#define CL_HPP_MINIMUM_OPENCL_VERSION 110


#pragma warning(disable : 4996 26439)


#include <ksn/stuff.hpp>
#include <ksn/window.hpp>
#include <ksn/graphics.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <GL/glew.h>
#include <CL/opencl.hpp>

#include <stdlib.h>

#include <numeric>
#include <math.h>
#include <execution>



//I'll need a bunch of those
//#pragma comment(lib, "")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")

#pragma comment(lib, "OpenCL.lib")
#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "runtimeobject.lib")

#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_x86_instruction_set.lib")





//template<typename... params_t>
//void nop(params_t&& ...params) {}


[[noreturn]] void блять(int code, const char* fmt = "", ...)
{
	va_list ap;
	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);

	va_end(ap);

	printf("\nPress enter\n");
	(void)getchar();
	exit(code);
}

FILE* print_fd = nullptr;
void print(const char* str)
{
	fwrite(str, sizeof(char), strlen(str), stdout);
	if (print_fd) 
		fwrite(str, sizeof(char), strlen(str), print_fd);
}



constexpr const char* cl_src_pixel_processor = R"(

struct parameters_t
{
	float center_x, center_y, scale, constant;
	unsigned int color_from, color_to, max_ticks, width, height;
};

__kernel void pixel_processor(__global unsigned int* pixels, __global struct parameters_t* p)
{
	unsigned int i = get_global_id(0) * get_local_size(0) + get_local_id(0);
	unsigned int j = get_global_id(1) * get_local_size(1) + get_local_id(1);

	unsigned n = 0;
	
	{
		float x0 = j - (float)p->height / 2;
		float y0 = i - (float)p->width / 2;

		float x = 0, y = 0, x2 = 0, y2 = 0;
		while (x2 + y2 <= 4 && n < p->max_ticks)
		{
			y = (x + x) * y + y0;
			x = x2 - y2 + x0;
			x2 = x * x;
			y2 = y * y;
			++n;
		}
	}
	
	{
		float t = (float)n / (float)p->max_ticks;
		unsigned int dc = p->color_to - p->color_from;
		pixels[i  *  p->height + j] = p->color_from + (unsigned int)(t * dc);
	}
	
}

)";

using src_t = const char*;
constexpr src_t cl_srcs[] = { cl_src_pixel_processor };

size_t cl_srcs_lengths[ksn::countof(cl_srcs)];



struct parameters_t
{
	float center_x, center_y, scale, constant;
	unsigned int color_from, color_to, max_ticks, width, height;
};



int main()
{
	for (size_t i = 0; i < ksn::countof(cl_srcs); ++i)
	{
		cl_srcs_lengths[i] = strlen(cl_srcs[i]);
	}


	constexpr static size_t io_buffer_size = 256;

	constexpr static size_t width = 800;
	constexpr static size_t height = 600;
	
	constexpr static float ratio = float(width) / height;

	constexpr static uint32_t color_from = 0xFF03BAFC, color_to = 0xFFFC7303;
	
	size_t temp;
	uint32_t max_ticks = 20;


	ksn::window_t win;
	ksn::window_t::error_t win_code = win.open(width, height, "", {},
		ksn::window_t::style::caption | ksn::window_t::style::close_button | ksn::window_t::style::border | ksn::window_t::style::hidden);

	if (win_code != ksn::window_t::error::ok)
		блять(1, "Failed to create window, code %i", (int)win_code);
	

	float center_x = 0;
	float center_y = 0;
	float scale = 0.010000001f;
	float constant = 1; //used to generate julia sets


	void* _p_screen_data = malloc(width * height * sizeof(uint32_t));
	if (_p_screen_data == nullptr) return 2;
	
	uint32_t(&screen_data)[height][width] = *(uint32_t(*)[height][width])_p_screen_data;
	memset(screen_data, 0, sizeof(screen_data));


	union
	{
		char buffer4k[4096];
		int8_t buffer4ki8[4096];
		uint8_t buffer4kui8[4096];
		int32_t buffer4ki32[1024];
		uint32_t buffer4kui32[1024];
	};
	void* buffer4kvoid = (void*)buffer4k;
	memset(buffer4k, 0, sizeof(buffer4k));


	static constexpr size_t platforms_static_max = 64;
	cl_platform_id platforms[platforms_static_max];
	uint8_t platforms_situable[platforms_static_max / 8];
	cl_uint platforms_count;
	clGetPlatformIDs(platforms_static_max, platforms, &platforms_count);
	
	if (platforms_count == 0) блять(1, "No OpenCL platforms found in the system");
	
	memset(platforms_situable, 0, sizeof(platforms_situable));
	for (cl_uint i = 0; i < platforms_count; ++i)
	{
		clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(buffer4k), buffer4k, &temp);

		int ver_maj = 0, ver_min = 0;

		auto set_platform_situable = [&]
		() -> void
		{
			if (ver_maj > 1 || ver_maj == 1 && ver_min >= 1)
			{
				uint8_t& byte = platforms_situable[i / 8];
				uint8_t value = 1 << (i % 8);
				byte |= value;
			}
		};

		if (
			sscanf(buffer4k, "OpenCL %i.%i", &ver_maj, &ver_min) == 2 ||
			sscanf(buffer4k, "%i.%i", &ver_maj, &ver_min) == 2
			)
		{
			set_platform_situable();
			continue;
		}

		char* p = buffer4k;
		//skip all non-digit in the buffer
		while (*p != '\0' && !isdigit(*p)) ++p;

		if (*p != '\0' && sscanf(buffer4k, "%i.%i", &ver_maj, &ver_min) == 2)
		{
			set_platform_situable();
			continue;
		}
	}

	if (std::accumulate(platforms_situable, platforms_situable + ksn::countof(platforms_situable), size_t(0)) == 0)
	{
		блять(1, "No situable OpenCL platform found");
	}


	setvbuf(stdout, nullptr, _IOFBF, io_buffer_size);
	
	print("Select OpenCL platform:\n");
	for (cl_uint i = 0; i < platforms_count; ++i)
	{
		if (platforms_situable[i / 8] & (1 << (i % 8)))
		{
			clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buffer4k), buffer4k, nullptr);
			printf("[%i]: ", int(i) + 1);
			print(buffer4k);
			clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buffer4k), buffer4k, nullptr);
			print(" by ");
			print(buffer4k);
			clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(buffer4k), buffer4k, nullptr);
			print(" on ");
			print(buffer4k);
			putchar('\n');
		}
	}
	
	//fflush(stdout);
	setvbuf(stdout, nullptr, _IONBF, io_buffer_size);

	int platform_index;

	rewind(stdin);
	do
	{
		print("Select: ");
		if (scanf("%i", &platform_index) != 1)
		{
			rewind(stdin);
			continue;
		}

		if (platform_index <= 0 || platform_index > (int)platforms_count) continue;
		--platform_index;

		if (platforms_situable[platform_index / 8] & (1 << (platform_index % 8))) break;
	} while (true);
	
	cl_uint num_devices = 0;
	clGetDeviceIDs(platforms[platform_index], CL_DEVICE_TYPE_ALL, 4096 / sizeof(cl_device_id), (cl_device_id*)buffer4k, &num_devices);
	
	cl_context_properties context_properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platform_index], 0 };

	temp = 0;
	cl_context context = clCreateContext(context_properties, num_devices, (cl_device_id*)buffer4k, nullptr, nullptr, (cl_int*)&temp);
	if (temp != 0 || context == nullptr) блять(1, "Failed to create OpenCL context, error %i", (int)temp);

	cl_program program = clCreateProgramWithSource(context, 1, (const char**)cl_srcs, cl_srcs_lengths, (cl_int*)&temp);
	if (clBuildProgram(program, num_devices, (cl_device_id*)buffer4k, "-cl-std=CL1.2", nullptr, nullptr))
	{
		print_fd = fopen("mbs_cl_build_log.txt", "w");
		setvbuf(stdout, nullptr, _IOFBF, 16384);
		for (cl_uint i = 0; i < num_devices; ++i)
		{
			auto& device = ((cl_device_id*)buffer4k)[i];
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS, sizeof(temp), &temp, nullptr);
			if (temp != CL_BUILD_SUCCESS)
			{
				char buffer[8000];
				size_t length = 0;
				clGetDeviceInfo(device, CL_DEVICE_NAME, 4000, buffer, nullptr);
				print("Device ");
				print(buffer);
				print(" has reported an OpenCL program build error\nBuild log:\n");
				clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 4000, buffer, &length);
				if (length == 4000) buffer[3999] = '\0';
				print(buffer);
				if (length == 4000)
				{
					print("\n\n(Build log was restricted to 4000 symbols)\n");
				}
				else
				{
					putchar('\n');
					putchar('\n');
				}
			}
		}
		fflush(stdout);
		setvbuf(stdout, nullptr, _IONBF, 0);
		if (print_fd)
		{
			fclose(print_fd);
			print_fd = nullptr;
			print("Build log saved to mbs_cl_build_log.txt");
		}
		блять(1);
	}
	

	cl_kernel kernel_pixel_processor = clCreateKernel(program, "pixel_processor", (cl_int*)&temp);
	if (kernel_pixel_processor == nullptr || temp != 0) блять(1, "Failed to create OpenCL kernel: error %i", (int)temp);
	cl_mem screen_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(screen_data), nullptr, nullptr);
	cl_mem parameter_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(parameters_t), nullptr, nullptr);
	
	parameters_t kernel1_params;
	kernel1_params.center_x = center_x;
	kernel1_params.center_y = center_y;
	kernel1_params.scale = scale;
	kernel1_params.color_from = color_from;
	kernel1_params.color_to = color_to;
	kernel1_params.max_ticks = max_ticks;
	kernel1_params.constant = constant;
	kernel1_params.width = width;
	kernel1_params.height = height;

	clSetKernelArg(kernel_pixel_processor, 0, sizeof(screen_buffer), &screen_buffer);
	clSetKernelArg(kernel_pixel_processor, 1, sizeof(kernel1_params), &kernel1_params);
	
	
	setvbuf(stdout, nullptr, _IOFBF, io_buffer_size);
	print("List of devices on the current platform:\n");
	for (cl_uint i = 0; i < num_devices; ++i)
	{
		cl_device_id device = ((cl_device_id*)buffer4k)[i];
		printf("[%i]: ", i + 1);
		char buffer[4096];
		clGetDeviceInfo(device, CL_DEVICE_NAME, 4096, buffer, &temp);
		print(buffer);
		print(" by ");
		clGetDeviceInfo(device, CL_DEVICE_VENDOR, 4096, buffer, &temp);
		print(buffer);
		putchar('\n');
	}
	setvbuf(stdout, nullptr, _IONBF, io_buffer_size);
	
	int device_id;
	rewind(stdin);
	while (1)
	{
		print("Select a device: ");

		if (scanf("%i", &device_id) != 1)
		{
			rewind(stdin);
			continue;
		}
		if (device_id <= 0 || device_id > (int)num_devices) continue;
		break;
	}


	cl_command_queue q = clCreateCommandQueue(context, ((cl_device_id*)buffer4k)[device_id], 0, nullptr);


	win.make_current();
	win.show();

	ksn::nop();

	bool changed = true;
	bool changed_secondary = false;
	bool add = false;
	bool sub = false;

	ksn::graphics::color_hsv_t fill_color(0, 100, 100);
	std::fill(std::execution::par_unseq, (uint32_t*)screen_data, (uint32_t*)screen_data + width * height, (uint32_t)ksn::graphics::color_t(fill_color));

	win.set_vsync_enabled(1);
	while (win.is_open())
	{
		constexpr static size_t global_work_offset[2] = { 0, 0};
		constexpr static size_t global_work_size[2] = { width, height };
		constexpr static size_t local_work_size[2] = { 16, 16 };

		if (changed)
		{
			clEnqueueWriteBuffer(q, parameter_buffer, CL_FALSE, 0, sizeof(kernel1_params), &kernel1_params, 0, nullptr, nullptr);
			clEnqueueNDRangeKernel(q, kernel_pixel_processor, 2, global_work_offset, global_work_size, local_work_size, 0, nullptr, nullptr);
			clEnqueueReadBuffer(q, screen_buffer, CL_TRUE, 0, sizeof(screen_data), &screen_data[0][0], 0, nullptr, nullptr);
			//clFlush(q);

			changed = false;
			changed_secondary = true;

			glDrawPixels(width, height, GL_BGRA, GL_UNSIGNED_BYTE, screen_data);
		}
		else
		{
			if (changed_secondary)
			{
				glDrawPixels(width, height, GL_BGRA, GL_UNSIGNED_BYTE, screen_data);
				changed_secondary = false;
			}
		}

		win.swap_buffers();
		

		bool changed_now = false;
		ksn::event_t ev;
		while (win.poll_event(ev))
		{
			if (ev.type == ksn::event_type_t::close) win.close();
			else if (ev.type == ksn::event_type_t::keyboard_press)
			{
				if (ev.keyboard_button_data.button == ksn::event_t::keyboard_button_t::esc) win.close();
			}

			if (ev.type == ksn::event_type_t::keyboard_press)
			{
				if (ev.keyboard_button_data.button == ksn::event_t::keyboard_button_t::add) add = true;
				else if (ev.keyboard_button_data.button == ksn::event_t::keyboard_button_t::substract) sub = true;
			}
			else if (ev.type == ksn::event_type_t::keyboard_release)
			{
				if (ev.keyboard_button_data.button == ksn::event_t::keyboard_button_t::add) add = false;
				else if (ev.keyboard_button_data.button == ksn::event_t::keyboard_button_t::substract) sub = false;
			}
		}

		if (add ^ sub)
		{
			if (add)
			{
				fill_color.hue = (fill_color.hue + 4) % 360;
				changed = changed_now = true;
			}
			else if (sub)
			{
				int16_t hue = fill_color.hue - 4;
				if (hue < 0) hue += 360;
				fill_color.hue = hue % 360;
				changed = changed_now = true;
			}
		}

		if (changed_now)
		{
			std::fill(std::execution::par_unseq, (uint32_t*)screen_data, (uint32_t*)screen_data + width * height, (uint32_t)ksn::graphics::color_t(fill_color));
		}
	}

	return 0;
}
