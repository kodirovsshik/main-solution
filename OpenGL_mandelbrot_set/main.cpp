
#define CL_HPP_TARGET_OPENCL_VERSION 110
#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_TARGET_OPENCL_VERSION CL_HPP_TARGET_OPENCL_VERSION
#define CL_MINIMUM_OPENCL_VERSION CL_HPP_MINIMUM_OPENCL_VERSION


#pragma warning(disable : 4996 26439 4530)


#include <ksn/stuff.hpp>
#include <ksn/window.hpp>
#include <ksn/graphics.hpp>
#include <ksn/opencl_kernel_tester.hpp>
#include <ksn/opencl_selector.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <GL/glew.h>
#include <CL/opencl.hpp>

#include <stdlib.h>

#include <numeric>
#include <math.h>



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
#pragma comment(lib, "libksn_opencl_selector.lib")





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

//FILE* print_fd = nullptr;
//void print(const char* str)
//{
//	fwrite(str, sizeof(char), strlen(str), stdout);
//	if (print_fd) 
//		fwrite(str, sizeof(char), strlen(str), print_fd);
//}



constexpr const char* cl_src_pixel_processor = R"(

struct parameters_t
{
	float center_x, center_y, scale, offset_const;
	unsigned int color_from, color_to, max_ticks, width, height;
};

__kernel void pixel_processor(__global unsigned int* pixels, __global struct parameters_t* p)
{
	unsigned int i = get_global_id(0);
	unsigned int j = get_global_id(1);

	unsigned n = 0;
	
	if (1)
	{
		float x = p->scale * (i + i - p->width) + p->center_x;
		float y = p->scale * (p->height - j - j) + p->center_y;
		
		float x2 = x*x;
		float y2 = y*y;
		
		//z <- z^2 + c
		//Decomposing:
		//(x + yi)^2 = (x^2 - y^2) + i*(2*x*y)
		//That is,
		//R <- R^2 + I^2 + Re(c)
		//I <- 2*R*I + Im(c)

		while (x2 + y2 <= 4 && n < p->max_ticks)
		{
			y *= x + x;
			x = x2 - y2 + 1;
			x2 = x * x;
			y2 = y * y;
		}
	}
	
	{
		float t = (float)n / (float)p->max_ticks;
		unsigned int dc = p->color_to - p->color_from;
		pixels[i  *  p->height + j] = p->color_from + (unsigned int)(t * dc);
	}
	
}

)";



struct parameters_t
{
	float center_x, center_y, scale, constant;
	unsigned int color_from, color_to, max_ticks, width, height;
};



namespace ksn_opencl_kernel_tester
{
	//__kernel void pixel_processor(__global unsigned int* pixels, __global struct parameters_t* p)
	//{
	//	uint64_t i = get_global_id(0) * get_local_size(0) + get_local_id(0);
	//	unsigned int j = get_global_id(1) * get_local_size(1) + get_local_id(1);

	//	unsigned n = 0;

	//	{
	//		float x0 = j - (float)p->height / 2;
	//		float y0 = i - (float)p->width / 2;

	//		float x = 0, y = 0, x2 = 0, y2 = 0;
	//		while (x2 + y2 <= 4 && n < p->max_ticks)
	//		{
	//			y = (x + x) * y + y0;
	//			x = x2 - y2 + x0;
	//			x2 = x * x;
	//			y2 = y * y;
	//			++n;
	//		}
	//	}

	//	{
	//		float t = (float)n / (float)p->max_ticks;
	//		unsigned int dc = p->color_to - p->color_from;
	//		pixels[i * p->height + j] = p->color_from + (unsigned int)(t * dc);
	//	}

	//}


}

using src_t = const char*;
constexpr src_t cl_srcs[] = { cl_src_pixel_processor };

size_t cl_srcs_lengths[ksn::countof(cl_srcs)];



int main()
{
	for (size_t i = 0; i < ksn::countof(cl_srcs); ++i)
	{
		cl_srcs_lengths[i] = strlen(cl_srcs[i]);
	}


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

	ksn::opencl_selector_data_t selector_data;
	selector_data.build_log_file_name = L"build_log.txt";
	selector_data.cl_sources = cl_srcs;
	selector_data.cl_sources_lengthes = cl_srcs_lengths;
	selector_data.cl_sources_number = ksn::countof(cl_srcs);
	temp = ksn::opencl_selector(&selector_data);
	if (temp != 0) return (int)temp;

	cl_context context = selector_data.context;
	cl_program program = selector_data.program;
	cl_command_queue q = selector_data.q;


	float center_x = 0;
	float center_y = 0;
	float scale = 0.003000001f;
	float constant = 1; //can be adjusted to generate julia sets


	void* _p_screen_data = malloc(width * height * sizeof(uint32_t));
	if (_p_screen_data == nullptr) return 2;

	uint32_t(&screen_data)[height][width] = *(uint32_t(*)[height][width])_p_screen_data;
	memset(screen_data, 0, sizeof(screen_data));


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
	clSetKernelArg(kernel_pixel_processor, 1, sizeof(parameter_buffer), &parameter_buffer);

	//FreeConsole();


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
		constexpr static size_t local_work_size[2] = { 1, 1 };

		if (changed)
		{
			auto d = ksn::measure_running_time_no_return([&]
				{
					clEnqueueWriteBuffer(q, parameter_buffer, CL_FALSE, 0, sizeof(kernel1_params), &kernel1_params, 0, nullptr, nullptr);
					clEnqueueNDRangeKernel(q, kernel_pixel_processor, 2, global_work_offset, global_work_size, nullptr, 0, nullptr, nullptr);
					//ksn_opencl_kernel_tester::call_kernel(2, global_work_offset, global_work_size, local_work_size, ksn_opencl_kernel_tester::pixel_processor, (uint32_t*)screen_data, &kernel1_params);
					clEnqueueReadBuffer(q, screen_buffer, CL_TRUE, 0, sizeof(screen_data), screen_data, 0, nullptr, nullptr);
					clFlush(q);
				});
			printf("T = %llu ms\n", d / 1000);

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
