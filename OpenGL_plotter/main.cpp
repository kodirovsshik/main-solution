
#include <ksn/window_gl.hpp>
#include <ksn/math_vec.hpp>

#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_window_gl.lib")
#pragma comment(lib, "libksn_time.lib")

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")

#pragma warning(disable : 5105 4005 5106)
#include <Windows.h>

import <GL/glew.h>;
import libksn.plotter.gl;

void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	static_assert(std::is_same_v<GLchar, char>);

	fprintf(stderr, "\ngl_error_callback() was called from:\n%.*s\n", (int)length, (const char*)message);
	__debugbreak();
}

template<class fp_t, class callable_t, class... args_t>
auto integrate(fp_t a, fp_t b, callable_t&& callable, args_t&& ...args)
{
	auto f = [&]
	(const fp_t& x)
	{
		return callable(x, std::forward<args_t>(args)...);
	};

	constexpr static size_t N = 1000;
	const fp_t dx = (b - a) / N;
	const fp_t dx_over2 = dx / 2;

	fp_t x = a;
	auto y_prev = f(x);

	decltype(y_prev) result = 0;

	size_t n = N;
	while (n --> 0)
	{
		x += dx;
		auto y = f(x);
		result += (y + y_prev) * dx_over2;
		y_prev = y;
	}

	return result;
}

template <class fp_t>
struct power
{
	fp_t val;
	power(fp_t x) : val(x) {}
	friend fp_t operator^(fp_t x, const power& y)
	{
		using std::pow;
		return pow(x, y.val);
	}
};

float dl(float x, float n)
{
	//float a = pow(x, n - 1) / pow(1 - pow(x, n), 1 - 1 / n);
	float a = (x ^ power(n - 1)) / ((1 - x ^ power(n)) ^ power(1 - 1 / n));
	return sqrtf(1 + a * a);
}

int main()
{
	constexpr uint32_t window_width = 1300;
	constexpr uint32_t window_height = 700;
	constexpr float screen_ratio = (float)window_height / window_width;

	constexpr float scroll_speed_horizontal = 0.05f;
	constexpr float scroll_speed_vertical = scroll_speed_horizontal / screen_ratio;


	float field_x_min = 0;
	float field_x_max = 5;
	float field_y_mid = 7;

	float field_x_range = field_x_max - field_x_min;
	float field_y_range;
	float field_y_max;
	float field_y_min;

	bool redraw;

	ksn::plotter_settings_t<float> plot_settings{};
	plot_settings.axis_enabled = true;
	plot_settings.axis_over_curve = true;

	auto lerp = [](int min, int max, float a, float b, uint16_t val)
	{
		return a + (b - a) / (max - min) * (val - min);
	};

	auto screen_to_field = [&]
	(uint16_t x, uint16_t y)
	{
		return ksn::vec2f{
			lerp(0, window_width - 1, field_x_min, field_x_max, x),
			lerp(window_height - 1, 0, field_y_min, field_y_max, y)
			};
	};

	auto recalculate_boundaries = [&]
	{
		field_x_max = field_x_min + field_x_range;

		field_y_range = field_x_range * screen_ratio;
		field_y_max = field_y_mid + field_y_range / 2;
		field_y_min = field_y_mid - field_y_range / 2;

		plot_settings.x1 = field_x_min;
		plot_settings.x2 = field_x_max;
		plot_settings.axis_x_min = field_x_min;
		plot_settings.axis_x_max = field_x_max;
		plot_settings.axis_y_min = field_y_min;
		plot_settings.axis_y_max = field_y_max;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(field_x_min, field_x_max, field_y_min, field_y_max, 0, 1);

		redraw = true;
	};


	ksn::window_style_t window_style{};
	window_style |= ksn::window_style::caption;
	window_style |= ksn::window_style::close_button;
	window_style |= ksn::window_style::hidden;

	ksn::window_gl_t::context_settings context_settings{};
	context_settings.ogl_debug = _KSN_IS_DEBUG_BUILD;
	context_settings.ogl_version_major = 4;
	context_settings.ogl_version_minor = 3;

	ksn::window_gl_t window;
	if (auto error = window.open(window_width, window_height, "", context_settings, window_style);
		error != ksn::window_open_result::ok)
	{
		return error;
	}

	window.set_framerate(60);
	window.context_make_current();

	recalculate_boundaries();

	if constexpr (_KSN_IS_DEBUG_BUILD)
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_error_callback, NULL);
	}

	glClearColor(0, 0, 0, 0);

	window.show();

	while (true)
	{
		if (redraw)
		{
			glClear(GL_COLOR_BUFFER_BIT);

			ksn::plot([]
			(float n)
				{
					return 4 * integrate<float>(1e-3f, 1, dl, n);
				}, plot_settings);

			window.swap_buffers();

			redraw = false;
		}

		window.tick();


		ksn::event_t ev;
		while (window.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				window.close();
				break;

			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::esc:
					window.close();
					break;
				}

			case ksn::event_type_t::mouse_scroll:
				if (GetAsyncKeyState(VK_MENU) < 0)
					break;

				if (GetAsyncKeyState(VK_CONTROL) < 0)
				{
					float zoom_factor = powf(0.8f, ev.mouse_scroll_data.delta);

					ksn::vec2f origin{ field_x_min, field_y_mid };
					ksn::vec2f zoom_point = screen_to_field(ev.mouse_scroll_data.x, ev.mouse_scroll_data.y);

					origin = zoom_point + (origin - zoom_point) * zoom_factor;
					field_x_min = origin[0];
					field_y_mid = origin[1];
					field_x_range *= zoom_factor;
				}
				else if (GetAsyncKeyState(VK_SHIFT) < 0)
					field_x_min -= field_x_range * scroll_speed_horizontal * ev.mouse_scroll_data.delta;
				else
					field_y_mid += field_y_range * scroll_speed_vertical * ev.mouse_scroll_data.delta;

				recalculate_boundaries();
				break;

			case ksn::event_type_t::mouse_release:
				if (ev.mouse_button_data.button == ksn::mouse_button_t::right)
				{
					auto pos = screen_to_field(ev.mouse_button_data.x, ev.mouse_button_data.y);
					printf("(%g, %g)\n", pos[0], pos[1]);
				}
			}
		}

		if (!window.is_open())
			break;
	}

}