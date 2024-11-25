
#include <GL/glew.h>

#include <ksn/window_gl.hpp>

#include <print>

#pragma comment(lib, "glew32s")
#pragma comment(lib, "opengl32")
#pragma comment(lib, "libksn_time")
#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_window_gl")

static constexpr size_t w = 1920;
static constexpr size_t h = 1080;

struct arr_holder
{
	struct rgb { uint8_t r, g, b; };

	uint8_t arr[h][w];
};

auto glString(GLenum x)
{
	return (const char*)glGetString(x);
}

int main()
{
	ksn::window_gl_t win;
	ksn::window_style_t style{};
	style |= ksn::window_style::default_style;

	auto arr_ptr = std::make_unique<arr_holder>();
	auto arr = arr_ptr->arr;

	win.open(w, h, "", {}, style);
	auto sz = win.get_client_size();
	auto pz = win.get_client_position();

	win.set_client_position(0, 0);

	win.context_make_current();
	win.set_framerate(win.get_monitor_framerate());

	std::println("OpenGL {}\n{}\n{}", glString(GL_VERSION), glString(GL_RENDERER), glString(GL_VENDOR));
	
	glClearColor(0, 0, 0, 0);

	while (win.is_open())
	{

		glClear(GL_COLOR_BUFFER_BIT);

		win.tick();

		ksn::event_t ev;
		while (win.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::escape:
					win.close();
					break;
				}
				break;
			}
		}
	}
}
