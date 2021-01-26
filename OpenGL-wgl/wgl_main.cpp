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

#include <ksn/fast_pimpl.hpp>



#pragma comment(lib, "opengl32.lib")



class window_t
{
private:

	static void _msg_worker(std::stop_token st, window_t* window)
	{
		MSG msg;
		while (!st.stop_requested() &&
			GetMessage(&msg, window->window, 0, 0) == 1)
		{
			window->msgs_lock.acquire();
			window->msgs.push_back(msg);
			window->msgs_lock.release();
		}
	}

	void _open(int width, int height, const wchar_t* name) noexcept
	{
		constexpr wchar_t wchars[] = L"qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";

		wchar_t class_name[32];
		for (auto& wch : class_name) wch = wchars[rand() % (sizeof(wchars) / sizeof(wchars[0]))];
		class_name[31] = 0;

		WNDCLASSW wc{};
		wc.lpfnWndProc = DefWindowProcW;
		wc.lpszClassName = class_name;
		//wc.hInstance = nullptr;

		int result = RegisterClassW(&wc);
		if (result == 0) exit(GetLastError());

		this->window = CreateWindowW
		(
			class_name,
			name,
			WS_CAPTION | WS_VISIBLE | WS_BORDER | WS_MINIMIZEBOX | WS_MINIMIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT,
			width, height,
			nullptr, nullptr, nullptr, 0
		);
		if (this->window == nullptr) exit(GetLastError());
		//SetWindowTextW(this->window, name);

		this->hdc = GetDC(this->window);

		PIXELFORMATDESCRIPTOR pfd{};
		//memset(&pfd, 0, sizeof(pfd));

		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.iLayerType = PFD_MAIN_PLANE;

		result = SetPixelFormat(this->hdc, ChoosePixelFormat(this->hdc, &pfd), &pfd);
		if (result == 0) exit(GetLastError());

		this->context = wglCreateContext(this->hdc);
		if (wglGetCurrentContext() == nullptr)
		{
			wglMakeCurrent(this->hdc, this->context);
		}

		this->msgs_thread = std::jthread(window_t::_msg_worker, this);
	}




public:
	uint64_t framerate_last_period;
	HWND window;
	HGLRC context;
	HDC hdc;
	uint32_t framerate_limit;
	std::jthread msgs_thread;
	std::deque<MSG> msgs;
	std::binary_semaphore msgs_lock;


public:

	window_t() = delete;
	window_t(int width, int height, const wchar_t* name = L"") noexcept
		: msgs_lock(1)
	{
		this->_open(width, height, name);
	}
	window_t(const window_t&) = delete;
	window_t(window_t&& w) noexcept
	{
		this->context = w.context;
		this->hdc = w.hdc;
		this->window = w.window;
		this->framerate_limit = w.framerate_limit;
		this->framerate_last_period = w.framerate_last_period;
		this->msgs_thread = std::move(w.msgs_thread);
		this->msgs_lock = std::move(w.msgs_lock);


		w.context = 0;
		w.hdc = 0;
		w.window = 0;
	}

	~window_t()
	{
		this->close();
	}




public:

	void open(int width, int height, const wchar_t* name = L"") noexcept
	{
		this->close();
		this->_open(width, height, name);
	}

	void close()
	{
		if (this->context) wglDeleteContext(this->context);
		if (this->hdc) ReleaseDC(this->window, this->hdc);
		if (this->window) DestroyWindow(this->window);
		this->msgs_thread.request_stop();
		this->msgs_thread.join();
	}



	void make_current()
	{
		wglMakeCurrent(this->hdc, this->context);
	}



	bool poll_event(MSG& msg)
	{
		bool result;
		this->msgs_lock.acquire();
		result = this->msgs.size() != 0;
		if (result) msg = this->msgs.pop_front();
		this->msgs_lock.release();
		return result;
	}



	void swap_buffers()
	{
		wglSwapLayerBuffers(this->hdc, WGL_SWAP_MAIN_PLANE);

		this->frame_end();
	}



	void frame_end()
	{
		using namespace std::chrono;
		using namespace std::this_thread;
		constexpr auto time_f = &system_clock::now;
		static auto last_time = time_f();

		auto now_time = time_f();

		auto last_period = (now_time - last_time);
		if (this->framerate_limit)
		{
			sleep_for(nanoseconds(1000000000 / this->framerate_limit) - last_period);
		}
		last_time = now_time;

		this->framerate_last_period = last_period.count();
	}



	//0 = unlimited
	void set_framerate_limit(uint32_t limit)
	{
		this->framerate_limit = limit;
	}
	//Framerate based on last frame process time
	uint32_t get_framerate()
	{
		return (uint32_t)(1000000000 / this->framerate_last_period);
	}
	//Last frame process time in nanoseconds
	uint64_t get_frame_last_period()
	{
		return this->framerate_last_period;
	}
};





int _main()
{
	constexpr int width = 800;
	constexpr int height = 600;
	constexpr int fps = 60;


	window_t win(width, height, L"GDI");
	win.set_framerate_limit(60);


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
