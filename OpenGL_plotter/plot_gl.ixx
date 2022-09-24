
export module libksn.plotter.gl;


import <ksn/ksn.hpp>;
import libksn.color;

import <GL/glew.h>;



_KSN_EXPORT_BEGIN

template<
	class fp_t = float, 
	color_some_rgb color_t = ksn::color_bgr_t
>
struct plotter_settings_t
{
	fp_t x1 = -2, x2 = 2;
	size_t steps = 1000;
	color_t color_real = color_rgb_t(0xFF0000);
	color_t color_imaginary = color_rgb_t(0xFF8000);
	color_t color_axis = color_rgb_t(0xFFFFFF);
	bool plot_real = true;
	bool plot_imaginary = false;
	bool use_lines = true;
	bool axis_enabled = false;
	bool axis_over_curve = false;
	fp_t axis_x_min = {};
	fp_t axis_x_max = {};
	fp_t axis_y_min = {};
	fp_t axis_y_max = {};
};

template<class fp_t = float, color_some_rgb color_t = color_rgb_t, class callable_t, class... args_t>
void plot(callable_t&& f, plotter_settings_t<fp_t, color_t> settings = {}, args_t&& ...args)
{
	auto draw_axis = [&]
	{
		glBegin(GL_LINES);
		glColor3ub(settings.color_axis.red(), settings.color_axis.green(), settings.color_axis.blue());
		glVertex2f(settings.axis_x_min, 0);
		glVertex2f(settings.axis_x_max, 0);
		glVertex2f(0, settings.axis_y_min);
		glVertex2f(0, settings.axis_y_max);
		glEnd();
	};

	if (settings.axis_enabled && !settings.axis_over_curve)
		draw_axis();

	if constexpr (true)
	{
		struct _gl_begin_sentry_t
		{
			_gl_begin_sentry_t(int type)
			{
				glBegin(type);
			}
			~_gl_begin_sentry_t()
			{
				glEnd();
			}
		} sentry1(settings.use_lines ? GL_LINE_STRIP : GL_POINTS);

		glColor3ub(settings.color_real.red(), settings.color_real.green(), settings.color_real.blue());


		fp_t dx = (settings.x2 - settings.x1) / settings.steps;
		fp_t x = settings.x1;

		size_t steps = settings.steps;
		while (steps-- > 0)
		{
			auto y = f(x, std::forward<args_t>(args)...);

			glVertex2d((double)x, (double)y);

			x += dx;
		}
	}

	if (settings.axis_enabled && settings.axis_over_curve)
		draw_axis();
}

_KSN_EXPORT_END
