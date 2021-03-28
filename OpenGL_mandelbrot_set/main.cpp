
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



constexpr const char cl_src_pixel_processor[] = R"(

struct data_t
{
	fp_t view_x, view_y;
	fp_t view_scale;
	fp_t bailout_r2; //should be >= 1000^2 for nice coloring
	fp_t epsilon;
	uint32_t color_begin;
	uint32_t color_delta;
	uint16_t max_iters;
};

uint32_t interpolate_color(data_t* data, float t)
{
	return data->color_begin + data->color_delta * fmod(t, 1);
}

void complex_multiply(__private fp_t* a1, __private fp_t* b1, fp_t a2, fp_t b2)
{
	fp_t new_real = *a1 * a2 - *b1 * b2;
	*b1 = *a1 * b2 + a2 * *b1;
	*a1 = new_real;
}

__kernel void pixel_processor(__global __read_only const data_t* data, __global __write_only uint32_t* screen)
{
	fp_t c_real = data->view_x + data->view_scale * get_global_id(0);
	fp_t c_imag = data->view_y + data->view_scale * get_global_id(1);
	
	fp_t z_real = c_real;
	fp_t z_imag = c_imag;
	
	fp_t der_real = 1;
	fp_t der_imag = 0;
	
	fp_t zr2 = z_real * z_real;
	fp_t zi2 = z_imag * z_imag;
	
	uint32_t color = 0; //Black
	
	fp_t pow = 1;
	
	for (size_t i = 0; i < data->max_iters; ++i)
	{
		fp_t der2 = der_real * der_real + der_imag * der_imag;
		if (der2 < data->epsilon)
		{
			break;
		}
		
		zr2 = z_real * z_real;
		zi2 = z_imag * z_imag;
		
		fp_t r2 = zr2 + zi2;
		if (r2 > data->bailout_r2)
		{
			color = interpolate_color(data, log(r2) / pow);
			break;
		}
		
		complex_multiply(&der_real, &der_imag, z_real, z_imag);
		
		//z = z^2 + c
		z_imag = z_imag * (z_real + z_real) + c_imag;
		z_real = zr2 - zi2 + c_real;
		
		pow += pow; //pow *= 2
	}
	
	screen[get_global_id(1) * get_global_size(0) + get_global_id(0)] = color;
}

)";


typedef float fp_t;

struct data_t
{
	fp_t view_x, view_y;
	fp_t view_scale;
	fp_t bailout_r2; //should be >= 1000^2 for nice coloring
	fp_t epsilon;
	uint32_t color_begin;
	uint32_t color_delta;
	uint16_t max_iters;
};




using src_t = const char*;
src_t cl_srcs[] = { cl_src_pixel_processor };

size_t cl_srcs_lengths[1];



int main()
{
	char buffer[ksn::countof(cl_src_pixel_processor) + 32];
	for (size_t i = 0; i < ksn::countof(cl_srcs); ++i)
	{
		cl_srcs_lengths[i] = strlen(cl_srcs[i]);
	}


	constexpr static size_t width = 800;
	constexpr static size_t height = 600;
	
	constexpr static float ratio = float(width) / height;

	constexpr static uint32_t color_from = 0x03BAFC, color_to = 0xFCA703;
	
	size_t temp;
	uint32_t max_ticks = 30;

	
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


	fp_t center_x = 0;
	fp_t center_y = 0;
	fp_t scale = fp_t(0.00500000L);
	//float constant = 1; //can be adjusted to generate julia sets


	void* _p_screen_data = malloc(width * height * sizeof(uint32_t));
	if (_p_screen_data == nullptr) return 2;

	uint32_t(&screen_data)[height][width] = *(uint32_t(*)[height][width])_p_screen_data;
	memset(screen_data, 0, sizeof(screen_data));


	cl_kernel kernel_pixel_processor = clCreateKernel(program, "pixel_processor", (cl_int*)&temp);
	if (kernel_pixel_processor == nullptr || temp != 0) блять(1, "Failed to create OpenCL kernel: error %i", (int)temp);
	cl_mem screen_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(screen_data), nullptr, nullptr);
	cl_mem parameter_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(data_t), nullptr, nullptr);
	
	data_t kernel1_params;
	kernel1_params.view_x = center_x - width * scale / 2;
	kernel1_params.view_y= center_y - height * scale / 2;
	kernel1_params.view_scale = scale;
	kernel1_params.color_begin = color_from;
	kernel1_params.color_delta = color_to - color_from;
	kernel1_params.max_iters = max_ticks;

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
				if (ev.keyboard_button_data.button == ksn::keyboard_button_t::esc) win.close();
			}

			if (ev.type == ksn::event_type_t::keyboard_press)
			{

			}
			else if (ev.type == ksn::event_type_t::keyboard_release)
			{

			}
		}

		//idk what this code does

		//if (add ^ sub)
		//{
		//	if (add)
		//	{
		//		fill_color.hue = (fill_color.hue + 4) % 360;
		//		changed = changed_now = true;
		//	}
		//	else if (sub)
		//	{
		//		int16_t hue = fill_color.hue - 4;
		//		if (hue < 0) hue += 360;
		//		fill_color.hue = hue % 360;
		//		changed = changed_now = true;
		//	}
		//}

		//if (changed_now)
		//{
		//	std::fill(std::execution::par_unseq, (uint32_t*)screen_data, (uint32_t*)screen_data + width * height, (uint32_t)ksn::graphics::color_t(fill_color));
		//}
	}

	return 0;
}
