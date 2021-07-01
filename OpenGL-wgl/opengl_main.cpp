
/*

I proudly declare this project an OpenGL learning zone.

*/



#include <ksn/ksn.hpp>
#include <ksn/window_gl.hpp>

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>

#define NOMINMAX
#include <Windows.h>

#include <stdint.h>

#include "shader_program.hpp"
#include "resource_manager.hpp"



#ifdef _KSN_COMPILER_MSVC
#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_window_gl.lib")
#pragma comment(lib, "libksn_time.lib")
#pragma comment(lib, "libksn_crc.lib")
#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_resource_manager.lib")

#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")
#endif



#define xassert(expr) { if (!(expr)) abort(); }



void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	static_assert(std::is_same_v<GLchar, char>);

	fprintf(stderr, "\ngl_error_callback() was called:\n%.*s\n", (int)length, (const char*)message);
	__debugbreak();
}



int main(int argc, char** argv)
{
	static constexpr uint16_t width = 800;
	static constexpr uint16_t height = 600;


	constexpr ksn::window_style_t window_style =
		ksn::window_style::close_button |
		ksn::window_style::caption | 
		ksn::window_style::hidden;
	
	ksn::window_gl_t::context_settings context_settings;
	context_settings.ogl_version_major = 4;
	context_settings.ogl_version_minor = 3;
	context_settings.ogl_compatibility_profile = false;
	context_settings.ogl_debug = true;

	ksn::window_gl_t window(width, height, "", context_settings, window_style);
	if (!window.is_open())
	{
		MessageBoxA(GetConsoleWindow(), "Failed to open the window", "Error", MB_ICONERROR);
		return 1;
	}
	window.set_framerate(60);



	window.context_make_current();
	glDebugMessageCallback(gl_error_callback, nullptr);



	resources::resource_manager_t resm;

	auto& shader_trivial = *resm.shader_load("trivial", "resources/shaders/trivial_vertex.glsl", "resources/shaders/trivial_fragment.glsl");
	xassert(&shader_trivial);
	xassert(shader_trivial);



	constexpr static float triangle_points[] =
	{
		0, 0.5f, 0,
		-0.5f, -0.5f, 0,
		0.5f, -0.5f, 0,
	};
	constexpr static float triangle_colors[] =
	{
		1, 0, 0,
		0, 0, 1,
		0, 1, 0
	};



	window.show();



	GLuint points_vbo = -1;
	glGenBuffers(1, &points_vbo);
	xassert(points_vbo != -1);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_points), triangle_points, GL_STATIC_DRAW);

	GLuint colors_vbo = -1;
	glGenBuffers(1, &colors_vbo);
	xassert(colors_vbo != -1);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_colors), triangle_colors, GL_STATIC_DRAW);


	GLuint vao = -1;
	glGenVertexArrays(1, &vao);
	xassert(vao != -1);
	glBindVertexArray(vao);


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	

	glClearColor(0, 1, 0, 0);

	while (window.is_open())
	{
		//Clear

		glClear(GL_COLOR_BUFFER_BIT);



		//Draw
		
		shader_trivial.use();

		glDrawArrays(GL_TRIANGLES, 0, 3);



		//Display and wait
		
		window.swap_buffers();
		window.tick();



		//Event handling

		ksn::event_t ev;
		while (window.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				window.close();
				break;


			case ksn::event_type_t::resize:
				glViewport(0, 0, ev.window_resize_data.width_new, ev.window_resize_data.height_new);
				break;


			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::esc:
					window.close();
					break;


				default:
					break;
				}
				break;


			default:
				break;
			}
		}
	}

}