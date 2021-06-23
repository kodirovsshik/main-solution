
#define CL_HPP_ENABLE_EXCEPTIONS 1
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120



//#include <CL/opencl.hpp>

#include <stdint.h>
#include <stdio.h>

#include <Windows.h>

//#include <ksn/stuff.hpp>

//#include <ksn/opencl_selector.hpp>


#pragma comment(lib, "Winmm.lib")

//#pragma comment(lib, "OpenCL.lib")
//#pragma comment(lib, "cfgmgr32.lib")
//#pragma comment(lib, "runtimeobject.lib")
// 
#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_window_gl.lib")
#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_x86_instruction_set.lib")
#pragma comment(lib, "libksn_time.lib")
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
#include <ksn/time.hpp>

#include <GL/glew.h>

void usleep(long long usec)
{
	static HANDLE timer = []
	{
		auto timer = CreateWaitableTimer(NULL, TRUE, NULL);   /* Timer handle */
		if (!timer) abort();
		return timer;
	}();
	static LARGE_INTEGER li;   /* Time defintion */

	li.QuadPart = -usec * 10;

	SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
	WaitForSingleObject(timer, INFINITE);
}


int main()
{
	static constexpr uint16_t width = 800;
	static constexpr uint16_t height = 600;
	static constexpr size_t screen_buffer_size = width * height * 3;
	static constexpr uint16_t framerate = 240;
	
	int ok;
	(void)ok;
	
	ksn::window_gl_t win;
	
	win.open(width, height, "");
	win.context_make_current();

	ksn_dynamic_assert(win.is_open() && win.context_is_current(), "");
	

	uint8_t* screen_buffer = new uint8_t[screen_buffer_size];
	ksn_dynamic_assert(screen_buffer, "");
	ksn_dynamic_assert(((uintptr_t)screen_buffer % sizeof(DWORD)) == 0, "");


	int color = 0;
	(void)color;

	win.tick();
	win.set_framerate(framerate);

	auto clock_f = std::chrono::high_resolution_clock::now;
	auto t1 = clock_f();
	
	uint64_t fps_time_counter = 0;
	int fps_time_counter_period = framerate / 3;
	int fps_avg_period = 10;
	float fps_avg = 0;

	uint32_t tick_counter = 1;


	using clock_t = std::chrono::high_resolution_clock;

	auto time_now = clock_t::now();
	auto time_last = clock_t::now();

	ksn::stopwatch sw;
	sw.start();

	glClearColor(0, 0, 0, 0);

	while (win.is_open())
	{
		//Clear screen
		memset(screen_buffer, 0, screen_buffer_size);




		glClear(GL_COLOR_BUFFER_BIT);
		//Draw everything
		win.draw_pixels_bgr_front(screen_buffer);
		win.swap_buffers();
		
		ksn::sleep_for(ksn::time::from_usec(1000000 / framerate) - sw.restart());

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

		sw.restart();

		

		//Frame limiting
		//time_now = clock_t::now();
		//auto work_time = std::chrono::duration_cast<std::chrono::microseconds>(time_now - time_last).count();

		//int64_t calculated_sleep_time = 1000000 / framerate - work_time;

		////std::this_thread::sleep_for(std::chrono::microseconds(calculated_sleep_time));
		//usleep(calculated_sleep_time);

		//time_last = clock_t::now();
		//auto sleep_time = duration_cast<std::chrono::microseconds>(time_last - time_now).count();



		//Debug output
		//if ((tick_counter % fps_time_counter_period) == 0)
		//{
		//	printf("Work: %i, Sleep1: %i, Sleep2: %i, Cycle: %i, over: %i\n", (int)work_time, (int)sleep_time, (int)calculated_sleep_time, int(work_time + sleep_time), int(sleep_time - calculated_sleep_time));
		//}
		//if (tick_counter == framerate) __debugbreak();



		//Minor stuff
		t1 = clock_f();

		tick_counter++;
	}


	//DeleteObject(bitmap);

	return 0;
}

