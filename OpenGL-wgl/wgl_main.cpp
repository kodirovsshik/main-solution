
#include <ksn/window.hpp>

#include <GL/glew.h>

#include <Windows.h>



#pragma comment(lib, "libksn_window.lib")

#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")



int main()
{

	ksn::window_t::context_settings ogl{ 3, 3, 24, false };
	ksn::window_t win;
	if (win.open(800, 600, "", ogl) != ksn::window_t::error::ok) return 1;
	win.make_current();

	static constexpr std::initializer_list<float> vbo_data = { -0.5f, -0.5f, 0, 0.5f, -0.5f, 0, 0.0f, 0.5f, 0 };

	GLuint vao, vbo;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vbo_data.size() * sizeof(float), vbo_data.begin(), GL_STATIC_DRAW);

	glClearColor(0, 0, 1, 0);

	win.set_vsync_enabled(true);
	while (win.is_open())
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(0);

		win.swap_buffers();

		ksn::event_t ev;
		while (win.poll_event(ev))
		{
			if (ev.type == ksn::event_type_t::close) win.close();
			else if (ev.type == ksn::event_type_t::keyboard_press && ev.keyboard_button_data.button == ksn::event_t::keyboard_button_t::esc) win.close();
		}
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	return 0;
}
