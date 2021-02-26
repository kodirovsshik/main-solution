#include <windows.h>

#include <assert.h>
#include <time.h>
#include <stdint.h>

#include <gl/GL.h>

#include <chrono>
#include <thread>
#include <algorithm>
#include <numeric>
#include <deque>
#include <semaphore>

#include <ksn/window.hpp>


#pragma comment(lib, "opengl32.lib")



int _main()
{
	constexpr int width = 800;
	constexpr int height = 600;
	constexpr int fps = 60;


	ksn::window_t win(width, height, L"GDI");
	//win.set_framerate_limit(60);


	uint8_t* pixel_data = new uint8_t[width * height * 3]{};



	while (1)
	{
		bool stop = false;
		int result; ((void)result);
		MSG msg;

		printf("a\n");
		while (win.poll_event(msg))
		{
			if (msg.message == WM_KEYDOWN)
			{
				if (msg.wParam == VK_ESCAPE) stop = true;
			}
		}
		printf("b\n");

		if (stop) break;

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
		win.swap_buffers();

		if constexpr (true)
		{
			static uint64_t max_time = 0;
			static time_t last_time = time(0);
			static uint32_t frames_elapsed = 0;

			auto current_time = win.get_frame_last_period();
			if (current_time > max_time) max_time = current_time;

			++frames_elapsed;

			if (time(0) > last_time)
			{
				uint32_t frames_lower_bound = uint32_t(1000000000 / max_time);
				if (frames_lower_bound > frames_elapsed) frames_lower_bound = frames_elapsed;
				printf("FPS: %03i/%03i\n", frames_elapsed, frames_lower_bound);

				last_time = time(0);
				frames_elapsed = 0;
				max_time = 0;
			}
		}
	}

	delete[] pixel_data;
	return 0;
}


int main()
{
	__try
	{
		return []() -> int
		{
			try
			{
				return _main();
			}
			catch (std::exception excp)
			{
				printf("\nUNHANDLED EXCEPTION: %s\n", excp.what());
				DebugBreak();
			}
			catch (...)
			{
				printf("\nUNHANDLED EXCEPTION\n");
				DebugBreak();
			}
			return -1;
		}
		();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("\nUNHANDLED SEH EXCEPTION %i:\n", GetExceptionCode());
		//printf("RBP = 0x%016llX\nRIP = 0x%016llX\n", GetExceptionInformation()->ContextRecord->Rsp, GetExceptionInformation()->ContextRecord->Rip);
	}
	return -1;
}
