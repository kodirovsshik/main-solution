#include <SFML/Graphics.hpp>

#include <ksn/math_complex.hpp>
#include <ksn/math_common.hpp>
#include <ksn/stuff.hpp>

#include <concepts>
#include <complex>

#include <math.h>

#pragma warning(disable : 4996 26451)

#pragma comment(lib, "sfml-system-s-d.lib")
#pragma comment(lib, "sfml-graphics-s-d.lib")
#pragma comment(lib, "sfml-window-s-d.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")


_KSN_BEGIN


template <class func_t, typename float_t, class... params_t>
concept funcR_t = std::is_convertible_v<std::invoke_result_t<func_t, float_t, params_t...>, float_t>;//&& std::is_floating_point_v<float_t>;



template <class func_t, typename float_t, class... params_t>
concept funcC_t =
(
	//type of func(arg, params...) is ksn::complex (possibly cv-qualified)
	std::is_same_v<ksn::complex<std::remove_cv_t<float_t>>,
	std::remove_cv_t<std::invoke_result_t<func_t, float_t&&, params_t&&...>>>
	||
	//or it is std::complex (also possibly cv-qualified)
	std::is_same_v<std::complex<std::remove_cv_t<float_t>>,
	std::remove_cv_t<std::invoke_result_t<func_t, float_t&&, params_t&&...>>>
	);



template <class callable_t, typename float_t, class... params_t>
concept func_t = funcR_t<callable_t, std::remove_cv_t<std::remove_reference_t<float_t>>, params_t&&...> || funcC_t<callable_t, std::remove_cv_t<std::remove_reference_t<float_t>>, params_t&&...>;






struct color_t
{
	uint8_t r, g, b;

	constexpr color_t() : r(0), g(0), b(0) {}
	constexpr color_t(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
	constexpr color_t(uint32_t x) : r(uint8_t(x >> 16)), g(uint8_t(x >> 8)), b(uint8_t(x)) {}

	color_t(sf::Color x) : r(x.r), g(x.g), b(x.b) {}



	operator sf::Color() const
	{
		return sf::Color(r, g, b);
	}



	static const color_t red;
	static const color_t green;
	static const color_t blue;
	static const color_t orange;
	static const color_t yellow;
	static const color_t black;
	static const color_t white;

};

constexpr const color_t color_t::red = color_t(0xFF0000);
constexpr const color_t color_t::green = color_t(0x00FF00);
constexpr const color_t color_t::blue = color_t(0x0000FF);
constexpr const color_t color_t::orange = color_t(0xFF7700);
constexpr const color_t color_t::yellow = color_t(0xFFFF00);
constexpr const color_t color_t::black = color_t(0x000000);
constexpr const color_t color_t::white = color_t(0xFFFFFF);




template<class _float_t = long double>
struct plotter
{

private:

	using my_t = plotter<_float_t>;
	using float_t = std::remove_cv_t<std::remove_reference_t<_float_t>>;



	static float_t y_get_real();

	template<same_to_cv<ksn::complex<float_t>> T>
	static float_t y_get_real(const T& obj)
	{
		return obj.real;
	}

	template<same_to_cv<std::complex<float_t>> T>
	static float_t y_get_real(const T& obj)
	{
		return obj.real();
	}

	template<same_to_cv<float_t> T>
	static float_t y_get_real(const T& obj)
	{
		return obj;
	}



	static float_t y_get_imag();

	template<same_to_cv<ksn::complex<float_t>> T>
	static float_t y_get_imag(const T& obj)
	{
		return obj.imag;
	}

	template<same_to_cv<std::complex<float_t>> T>
	static float_t y_get_imag(const T& obj)
	{
		return obj.imag();
	}

	template<same_to_cv<float_t> T>
	static float_t y_get_imag(const T&)
	{
		return float_t(0);
	}




#define y_move_real y_get_real
#define y_move_imag y_get_imag





public:

	template<bool draw_real, bool draw_imag, class callable_t, class... params_t>
	void plot(sf::RenderTexture& t, size_t w0, size_t w1, size_t h0, size_t h1, const float_t& x0, const float_t& x1, const float_t& y0, const float_t& y1, callable_t&& f, params_t&&... params)
	{
		using result_t = std::invoke_result_t<callable_t&&, float_t&&, params_t&&...>;

		float_t dx = x1 - x0;
		float_t dy = y1 - y0;

		size_t dw = w1 - w0;
		size_t dh = h1 - h0;

		float_t hx = dx / dw;
		float_t hy = dy / dh;

		const float_t* prev_hx = custom_hx;
		if (this->custom_hx == nullptr) this->custom_hx = &hx;

		const float_t& custom_hx = *this->custom_hx;



		auto draw_axis = [&]
		() -> void
		{
			static const float_t axis_radius = (float_t)this->axis_thickness / 2;
			sf::RectangleShape axis;

			//OX
			//axis.setPosition(0, float(dh + y0 / hy - 0.25));
			axis.setPosition(0, float(h1 + y0 / hy - 0.25));
			axis.setOrigin(0, (float)axis_radius + 0.25f);
			axis.setSize(sf::Vector2f{ (float)dw, (float)this->axis_thickness + 0.5f });
			axis.setFillColor(this->axis_color);

			t.draw(axis);

			//OY
			//axis.setPosition(float(dw + x0 / hx - 0.25), 0);
			axis.setPosition(float(w0 - x0 / hx - 0.25), 0);
			axis.setOrigin((float)axis_radius + 0.25f, 0);
			axis.setSize(sf::Vector2f{ (float)this->axis_thickness + 0.5f, (float)dh });
			axis.setFillColor(this->axis_color);

			t.draw(axis);
		};

		auto set_point = [&]
		(const float_t& height, float x_pixel, color_t color)
		{
			static const float curve_radius = this->curve_thickness / 2.f;

			sf::CircleShape dot;
			dot.setPosition(x_pixel - 0.25f, (float)height - 0.25f);
			dot.setOrigin(curve_radius, curve_radius);
			dot.setRadius(float(curve_radius + 0.25));
			dot.setFillColor(color);

			t.draw(dot);
		};

		auto set_line_consecutive = [&]
		(const float_t& height0, const float_t& height, size_t x_pixel2, color_t color)
		{
			static const float curve_radius = this->curve_thickness / 2.f;

			sf::RectangleShape line;
			
			line.setPosition(float(x_pixel2 - 1), (float)height0);
			line.setSize(sf::Vector2f((float)std::hypot(height - height0, 1) + curve_thickness, curve_thickness));
			line.setOrigin(curve_thickness / 2.f, curve_thickness / 2.f);
			line.setRotation((float)std::atan(height - height0) * 180.f / KSN_PIf);
			line.setFillColor(color);

			t.draw(line);
		};

		auto set_line_far = [&]
		(const float_t& height0, const float_t& height, size_t x_pixel0, size_t x_pixel, color_t color)
		{
			static const float curve_radius = this->curve_thickness / 2.f;

			sf::RectangleShape line;

			float dy = float(height - height0);
			float dx = float(x_pixel - x_pixel0);

			line.setPosition((float)x_pixel0, (float)height0);
			line.setSize(sf::Vector2f((float)std::hypot(dx, dy) + curve_thickness, curve_thickness));
			line.setOrigin(curve_thickness / 2.f, curve_thickness / 2.f);
			line.setRotation((float)std::atan2(dy, dx) * 180.f / KSN_PIf);
			line.setFillColor(color);

			t.draw(line);
		};



		if (this->axis_enabled && !this->axis_is_over_graph)
		{
			draw_axis();
		}



		if constexpr (draw_real && draw_imag)
		{
			std::vector<result_t> data;
			data.reserve(dw);
			float_t x = x0;

			for (; x <= x1; x += custom_hx)
			{
				data.emplace_back(std::invoke<callable_t>(std::forward<callable_t>(f), std::forward<float_t>(x), std::forward<params_t>(params)...));
			}

			auto render_precalculated_with_dots = [&]
				<std::invocable<const result_t&, params_t&&...> getter_t>
				(getter_t && getter, color_t color) -> void
			{
				float x_pixel = (float)w0;
				float_t value;
				for (const result_t& y : data)
				{
					value = getter(y);
					value = dh - (value - y0) / hy;

					set_point((float)value, x_pixel, color);
					x_pixel += float(custom_hx) / float(hx);
				}
			};

			auto render_precalculated_with_lines = [&]
				<std::invocable<const result_t&, params_t&&...> getter_t>
				(getter_t && getter, color_t color)
			{
				float x_pixel = (float)w0;
				float_t value, temp;

				if (data.size())
				{
					value = h1 - (getter(data.front()) - y0) / hy;
					set_point(value, x_pixel, color);
					for (size_t i = 1; i < data.size(); ++i)
					{
						temp = h1 - (getter(data[i]) - y0) / hy;
						set_line_consecutive(value, temp, i, color);
						value = temp;
					}
				}
			};

			if (this->curve_use_dots) _KSN_LIKELY
			{
				if (this->curve_imag_on_top) _KSN_UNLIKELY
				{
					render_precalculated_with_dots(&my_t::y_get_real<result_t>, this->curve_color_real);
					render_precalculated_with_dots(&my_t::y_get_imag<result_t>, this->curve_color_imag);
				}
				else _KSN_LIKELY
				{
					render_precalculated_with_dots(&my_t::y_get_imag<result_t>, this->curve_color_imag);
					render_precalculated_with_dots(&my_t::y_get_real<result_t>, this->curve_color_real);
				}
			}
			else _KSN_UNLIKELY
			{
				if (this->curve_imag_on_top) _KSN_UNLIKELY
				{
					render_precalculated_with_lines(&my_t::y_get_real<result_t>, this->curve_color_real);
					render_precalculated_with_lines(&my_t::y_get_imag<result_t>, this->curve_color_imag);
				}
				else _KSN_LIKELY
				{
					render_precalculated_with_lines(&my_t::y_get_imag<result_t>, this->curve_color_imag);
					render_precalculated_with_lines(&my_t::y_get_real<result_t>, this->curve_color_real);
				}
			}
		}
		else if constexpr (draw_real || draw_imag)
		{
			float_t x = x0;
			//Second condition is here in case we are requested to draw with lines but we have only one point so we can not do that
			if (this->curve_use_dots)
			{
				result_t y;
				float x_pixel = float(w0) /*+ float(x0) / float(hx)*/;
				for (; x <= x1; x += custom_hx)
				{
					y = std::invoke(f, std::forward<float_t>(x), std::forward<params_t>(params)...);
					if constexpr (draw_real)
					{
						set_point(dh - (my_t::y_get_real(y) - y0) / hy, x_pixel, this->curve_color_real);
					}
					else
					{
						set_point(dh - (my_t::y_get_imag(y) - y0) / hy, x_pixel, this->curve_color_imag);
					}

					if (&custom_hx == &hx)
					{
						x_pixel += 1;
					}
					else
					{
						x_pixel += float(custom_hx) / float(hx);
					}
				}
			}
			else
			{
				result_t y = std::invoke(f, std::forward<float_t>(x), std::forward<params_t>(params)...);
				result_t y_prev = y;

				auto render_with_lines = [&]
					<class getter_t>
					(getter_t && getter, color_t color)
				{
					//set first point
					result_t h_prev = dh - getter(y) / hy, h;
					set_point(h_prev, (float)w0, color);

					x += custom_hx;

					float x_pixel = (float)w0 + float(custom_hx) / float(hx), x_pixel0 = (float)w0;
					for (; x <= x1; x += custom_hx)
					{
						h = dh - getter(std::invoke(f, std::forward<float_t>(x), std::forward<params_t>(params)...)) / hy;
						if (&custom_hx == &hx) _KSN_LIKELY
						{
							set_line_consecutive(h_prev, h, (size_t)x_pixel, color);
							x_pixel += 1;
						}
						else _KSN_UNLIKELY
						{
							set_line_far(h_prev, h, (size_t)x_pixel0, (size_t)x_pixel, color);
							x_pixel += float(custom_hx) / float(hx);
						}
					}
				};

				//set first point
				y -= y0;

				if constexpr (draw_real)
					render_with_lines(my_t::y_get_real<float_t>, this->curve_color_real);
				else
					render_with_lines(my_t::y_get_imag<float_t>, this->curve_color_imag);
			}
		}



		if (this->axis_enabled && this->axis_is_over_graph)
		{
			draw_axis();
		}



		this->custom_hx = prev_hx;
	}



	sf::Font* marks_font = nullptr;
	const float_t* custom_hx = nullptr;
	color_t curve_color_real = color_t::green;
	color_t curve_color_imag = color_t::orange;
	color_t axis_color = color_t::white;
	color_t marks_color = color_t::white;
	uint8_t curve_thickness = 5;
	uint8_t axis_thickness = 3;
	bool curve_use_dots : 1 = true;
	bool curve_imag_on_top : 1 = false;
	bool axis_is_over_graph : 1 = false;
	bool axis_enabled : 1 = false;
	bool marks_enabled : 1 = false;



	//template<class callable_t, class... params_t> requires func_t<callable_t, float_t, params_t&&...>
	//void get_real_graph_texture(sf::RenderTexture& t, callable_t function, size_t width, size_t height, const float_t& x0, const float_t& x1, const float_t& y0, const float_t& y1, params_t&&... params)
	//{
	//	return this->plot<callable_t&&, true, false, params_t&&...>
	//		(t, std::forward<callable_t&&>(function), width, height, x0, x1, y0, y1, std::forward<params_t&&>(params)...);
	//}

	//template<class callable_t, class... params_t> requires func_t<callable_t, float_t, params_t&&...>
	//void get_imaginary_graph_texture(sf::RenderTexture& t, callable_t function, size_t width, size_t height, const float_t& x0, const float_t& x1, const float_t& y0, const float_t& y1, params_t&&... params)
	//{
	//	return this->plot<callable_t&&, false, true, params_t&&...>
	//		(t, std::forward<callable_t&&>(function), width, height, x0, x1, y0, y1, std::forward<params_t&&>(params)...);
	//}

	//template<class callable_t, class... params_t> requires func_t<callable_t, float_t, params_t&&...>
	//void get_merged_graph_texture(sf::RenderTexture& t, callable_t function, size_t width, size_t height, const float_t& x0, const float_t& x1, const float_t& y0, const float_t& y1, params_t&&... params)
	//{
	//	return this->plot<callable_t&&, true, true, params_t&&...>
	//		(t, std::forward<callable_t&&>(function), width, height, x0, x1, y0, y1, std::forward<params_t&&>(params)...);
	//}
};




_KSN_END



ksn::complex<double> T(double base, const double height)
{
	double t;

	static constexpr auto W = [](double y)
	{
		double result = NAN;
		ksn::newthon_method([&](double x) { return x * exp(x) - y; }, result, 0.7 * log(y + 1), 0);
		return result;
	};

	if (height == INFINITY)
	{
		t = log(1 / base);
		return W(t) / t;
	}

	double h_f;
	double h_i1;

	h_f = modf(height, &h_i1);
	if (h_f < 0)
	{
		h_f++;
		h_i1--;
	}

	int64_t h_i = (int64_t)h_i1;
	if (h_f == 0 && h_i < -1)
	{
		if ((h_i % 2) == 0)
			return ksn::complex<double>(-INFINITY);
		else
			return ksn::complex<double>(INFINITY);
	}

	ksn::complex<double> result = pow(base, h_f);

	if (h_i > 0)
	{
		while (h_i--)
		{
			result = pow(base, result);
		}
	}
	else if (h_i < 0)
	{
		while (h_i++)
		{
			result = log(result) / log(base);
		}
	}

	return result;
}

using ld = long double;

#define NOP() {int _ = 0; }; void()

int main()
{
	static constexpr long double x_range = 6;
	static constexpr long double x0 = -3;
	static constexpr long double y0 = -1;

	static constexpr size_t width = 1280, height = 720;
	static constexpr double ratio = (long double)width / height;

	static constexpr long double x1 = x0 + x_range;
	static constexpr long double y1 = y0 + x_range / ratio;

	static constexpr long double tetration_base = KSN_E;


	sf::RenderTexture t;
	t.create(width, height);

	ksn::plotter<double> g;
	g.curve_thickness = 2;
	g.curve_use_dots = false;

	g.axis_enabled = true;
	g.axis_thickness = 3;

	g.plot<true, true>(t, 0, width, 0, height, x0, x1, y0, y1, [](double x) { return T(tetration_base, x); });

	t.display();
	sf::Sprite spr_graph1(t.getTexture());



	sf::RenderWindow window(sf::VideoMode(width, height, 24), "SFML");
	window.setFramerateLimit(10);

	while (1)
	{
		sf::Event ev;
		while (window.pollEvent(ev))
		{
			switch (ev.type)
			{
			case sf::Event::Closed:

				window.close();

				break;


			case sf::Event::KeyPressed:
				if (ev.key.code == sf::Keyboard::Escape) window.close();
				
				break;


			default:

				break;
			}
		}

		if (!window.isOpen()) break;

		window.clear(ksn::color_t::black);
		window.draw(spr_graph1);
		window.display();
	}

	return 0;
}
