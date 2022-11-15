
#include <ksn/window_gl.hpp>
#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_window_gl")
#pragma comment(lib, "libksn_time")

#include <GL/glew.h>
#pragma comment(lib, "glew32s")
#pragma comment(lib, "opengl32")

#include <random>
#include <span>
#include <string>

template<std::floating_point T>
void make_noise(T* arr, size_t n, uint32_t x, uint32_t y)
{
	std::minstd_rand engine;
	std::uniform_real_distribution<T> dist(0, 1);

	std::span<T> _debug(arr, n);
	

	T amp = T(1);
	for (int i = 0; i < 24; ++i)
	{

	}
}

int main()
{
	ksn::window_gl_t::context_settings settings{};
	settings.ogl_version_major = 4;
	settings.ogl_version_minor = 3;
	settings.ogl_debug = true;

	ksn::window_style_t style{};
	style |= ksn::window_style::hidden;

	ksn::window_gl_t win;
	if (win.open(200, 200, "", settings, style) != ksn::window_open_result::ok)
		return -1;

	win.set_framerate(10);
	win.context_make_current();
	win.set_fullscreen_windowed();
	win.show();

	const uint16_t width = win.get_client_width();
	const uint16_t height = win.get_client_height();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
	glViewport(0, 0, width, height);

	std::vector<float> noise(win.get_client_width(), 0);
	//make_noise(noise.data(), noise.size(), 0);
	{

	}

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_LINES);
	glColor3f(1, 1, 1);
	for (size_t i = 0; i < width; ++i)
	{
		glVertex2f((float)i, 0);
		glVertex2f((float)i, noise[i] * height);
	}
	glEnd();

	win.swap_buffers();


	while (win.is_open())
	{
		//...


		ksn::event_t ev;
		while (win.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::esc:
					win.close();
					break;
				}
				break;
			}
		}

		win.tick();
	}
}
