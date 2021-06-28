
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



#ifdef _KSN_COMPILER_MSVC
#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_window_gl.lib")
#pragma comment(lib, "libksn_time.lib")

#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")
#endif



int main(int argc, char** argv)
{

	static constexpr uint16_t width = 800;
	static constexpr uint16_t height = 600;


	constexpr ksn::window_style_t window_style =
		ksn::window_style::close_button |
		ksn::window_style::caption;
	
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




	while (window.is_open())
	{



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