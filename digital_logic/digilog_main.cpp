
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





int wrapped_main(int argc, char** argv)
{

	std::error_code ec;
	if (!std::filesystem::exists("digilog_data", ec))
	{
		if (!ec)
			std::filesystem::create_directory("digilog_data", ec);
	}

	if (ec)
		fprintf(stderr, "Failed to access folder digilog_data/");



	init_opencl();



	ksn::window_gl_t::context_settings context_settings{};

	static constexpr ksn::window_style_t window_style = ksn::window_style::border | ksn::window_style::caption | ksn::window_style::close_min_max;
	
	ksn::window_open_result_t window_open_result = window.window.open(window.size.first , window.size.second, L"", context_settings, window_style);
	critical_assert1(window_open_result == ksn::window_open_result::ok, -1, "Fatal error", "Failed to open a window, GetLastError() = %i, result = %i", GetLastError(), window_open_result);

	draw_adapter.resize(window.size.first, window.size.second);



	int digilog_main_loop();
	return digilog_main_loop();
}
