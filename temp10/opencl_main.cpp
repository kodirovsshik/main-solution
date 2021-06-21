
#define CL_HPP_ENABLE_EXCEPTIONS 1
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120



//#include <CL/opencl.hpp>

#include <stdint.h>
#include <stdio.h>

#include <Windows.h>

//#include <ksn/stuff.hpp>

//#include <ksn/opencl_selector.hpp>


//#pragma comment(lib, "OpenCL.lib")
//#pragma comment(lib, "cfgmgr32.lib")
//#pragma comment(lib, "runtimeobject.lib")
// 
#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_window_gl.lib")
#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_x86_instruction_set.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")


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


#include <ksn/window.hpp>
#include <ksn/window_gl.hpp>
#include <ksn/stuff.hpp>

#include <GL/glew.h>



int main()
{
	static constexpr uint16_t width = 800;
	static constexpr uint16_t height = 600;
	static constexpr size_t screen_buffer_size = width * height * 3;
	
	
	int ok;
	(void)ok;
	

	
	ksn::window_gl_t win;
	win.open(width, height, "", ksn::window_gl_t::opengl_no_context);
	win.context_create();
	win.context_make_current();

	ksn_dynamic_assert(win.is_open() && win.context_is_current(), "");
	

	uint8_t* screen_buffer = new uint8_t[screen_buffer_size];
	ksn_dynamic_assert(screen_buffer, "");
	ksn_dynamic_assert(((uintptr_t)screen_buffer % sizeof(DWORD)) == 0, "");


	int color = 0;
	(void)color;

	win.tick();
	win.set_framerate(60);

	auto clock_f = std::chrono::high_resolution_clock::now;
	auto t1 = clock_f();
	
	uint64_t fps_time_counter = 0;
	int fps_time_counter_period = 10;

	size_t tick_counter = 0;

	while (win.is_open())
	{
		//Clear screen
		memset(screen_buffer, 0, screen_buffer_size);




		//Draw everything
		win.draw_pixels_bgr_front(screen_buffer);
		

		//Limit framerate
		win.tick();


		//Measure frame time
		auto t2 = clock_f();
		fps_time_counter += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();


		//Process events
		ksn::event_t ev;
		while (win.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				win.close();
				break;

			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::esc:
					win.close();
					break;
				}
			}
		}


		//Debug output
		if ((tick_counter % fps_time_counter_period) == 0)
		{
			printf("FPS = %.1f\n", 1e9 / fps_time_counter * fps_time_counter_period);
			fps_time_counter = 0;
		}



		//Minor stuff
		t1 = clock_f();

		tick_counter++;
	}


	//DeleteObject(bitmap);

	return 0;
}

