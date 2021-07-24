
#include <stdint.h>
#include <stdio.h>

#include <filesystem>
#include <system_error>

#include <ksn/window.hpp>

#define NOMINMAX
#include <Windows.h>

#include "window.hpp"
#include "err_handling.hpp"
#include "opencl.hpp"
#include "graphics.hpp"





void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	static_assert(std::is_same_v<GLchar, char>);

	fprintf(stderr, "\ngl_error_callback() was called:\n%.*s\n", (int)length, (const char*)message);
	__debugbreak();
}

int wrapped_main(int argc, char** argv)
{

	ksn::window_gl_t::context_settings context_settings{};
	context_settings.bits_per_color = 24;
	context_settings.ogl_compatibility_profile = true;
	context_settings.ogl_debug = _KSN_IS_DEBUG_BUILD;
	context_settings.ogl_version_major = 3;
	context_settings.ogl_version_minor = 0;

	static constexpr ksn::window_style_t window_style = ksn::window_style::border | ksn::window_style::caption | ksn::window_style::close_button | ksn::window_style::hidden;
	
	ksn::window_open_result_t window_open_result = window.window.open(window.size.first , window.size.second, L"", context_settings, window_style);
	critical_assert1(window_open_result == ksn::window_open_result::ok, -1, "Fatal error", "Failed to open a window, GetLastError() = %i, result = %i", GetLastError(), window_open_result);

	window.window.context_make_current();

	if constexpr (_KSN_IS_DEBUG_BUILD)
	{
		glDebugMessageCallback(gl_error_callback, nullptr);
	}





	init_opencl();





	draw_adapter.resize(window.size.first, window.size.second);





	std::error_code ec;
	if (!std::filesystem::exists("digilog_data", ec))
	{
		if (!ec)
			std::filesystem::create_directory("digilog_data", ec);
	}

	if (ec)
		fprintf(stderr, "Failed to access folder digilog_data/");





	window.window.show();

	int digilog_main_loop();
	return digilog_main_loop();
}
