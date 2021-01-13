#include <SFML/Graphics.hpp>

#include <ksn/math_complex.hpp>
#include <ksn/math_common.hpp>

#include <concepts>
#include <complex>

#include <math.h>

#pragma warning(disable : 4996)

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



template<class T, class U>
concept mb_cv_qualified = std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<U>>;



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





template<class float_t>
constexpr float_t fast_atan(const float_t& x) noexcept
{
	if (x < 0) return -fast_atan(-x);
	if (x < 0.68) return x * 0.9;
	if (x < 1.37) return x * 0.5 + 0.27;
	if (x < 2.3) return x * 0.24 + 0.62;
	if (x < 4) return x * 0.1 + 0.93;
	return KSN_PI / 2 - 1 / x;
};



template<class _float_t = long double>
struct grapher
{

private:

	using my_t = grapher<_float_t>;
	using float_t = std::remove_cv_t<std::remove_reference_t<_float_t>>;



	static float_t y_get_real();

	template<mb_cv_qualified<ksn::complex<float_t>> T>
	static float_t y_get_real(const T& obj)
	{
		return obj.real();
	}

	template<mb_cv_qualified<std::complex<float_t>> T>
	static float_t y_get_real(const T& obj)
	{
		return obj.real();
	}

	template<mb_cv_qualified<float_t> T>
	static float_t y_get_real(const T& obj)
	{
		return obj;
	}



	static float_t y_get_imag();

	template<mb_cv_qualified<ksn::complex<float_t>> T>
	static float_t y_get_imag(const T& obj)
	{
		return obj.imag();
	}

	template<mb_cv_qualified<std::complex<float_t>> T>
	static float_t y_get_imag(const T& obj)
	{
		return obj.imag();
	}

	template<mb_cv_qualified<float_t> T>
	static float_t y_get_imag(const T&)
	{
		return float_t(0);
	}




#define y_move_real y_get_real
#define y_move_imag y_get_imag





public:

	template<bool draw_real, bool draw_imag, class callable_t, class... params_t>
	void graph(sf::RenderTexture& t, size_t w0, size_t w1, size_t h0, size_t h1, const float_t& x0, const float_t& x1, const float_t& y0, const float_t& y1, callable_t&& f, params_t&&... params)
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
			static const float_t axis_radius = this->axis_thickness / 2;
			sf::RectangleShape axis;

			//OX
			axis.setPosition(0, float(dh + y0 / hy - 0.25));
			axis.setOrigin(0, (float)axis_radius + 0.25f);
			axis.setSize(sf::Vector2f{ (float)dw, (float)this->axis_thickness + 0.5f });
			axis.setFillColor(this->axis_color);

			t.draw(axis);

			//OY
			axis.setPosition(float(dw * x0 / dx - 0.25), 0);
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

			//Approx for sqrt(height^2 + 1)
			line.setSize(sf::Vector2f{ 1, float(height) > 0.8857f ? float(height) + 0.4f / float(height) : float(height) * 0.42f + 0.95f });

			t.draw(line);
		};

		auto set_line_far = [&]
		(const float_t& height0, const float_t& height, size_t x_pixel0, size_t x_pixel, color_t color)
		{
			static const float curve_radius = this->curve_thickness / 2.f;

			sf::RectangleShape line;
			line.setPosition(float(x_pixel0), (float)height0);

			line.setSize(sf::Vector2f{ 1, (float)sqrt(pow(height - height0, 2) + pow(x_pixel - x_pixel0, 2)) });

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
				data.emplace_back(std::invoke<callable_t>(f, std::forward<float_t>(x), std::forward<params_t>(params)...));
			}

			auto render_precalculated_with_dots = [&]
				<std::invocable<const result_t&, params_t&&...> getter_t>
				(getter_t && getter, color_t color) -> void
			{
				float_t value;
				float x_pixel = w0 + float(x0) / float(hx);
				for (const result_t& y : data)
				{
					value = getter(y);
					value = dh - (value - y0) / hy;

					set_point(x_pixel, value, color);

					if (&custom_hx == &hx)
					{
						x_pixel += 1;
					}
					else
					{
						x_pixel += float(custom_hx) / float(hx);
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
				float x_pixel = float(w0) + float(x0) / float(hx);
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



	template<class callable_t, class... params_t> requires func_t<callable_t, float_t, params_t&&...>
	void get_real_graph_texture(sf::RenderTexture& t, callable_t function, size_t width, size_t height, const float_t& x0, const float_t& x1, const float_t& y0, const float_t& y1, params_t&&... params)
	{
		return this->graph<callable_t&&, true, false, params_t&&...>
			(t, std::forward<callable_t&&>(function), width, height, x0, x1, y0, y1, std::forward<params_t&&>(params)...);
	}

	template<class callable_t, class... params_t> requires func_t<callable_t, float_t, params_t&&...>
	void get_imaginary_graph_texture(sf::RenderTexture& t, callable_t function, size_t width, size_t height, const float_t& x0, const float_t& x1, const float_t& y0, const float_t& y1, params_t&&... params)
	{
		return this->graph<callable_t&&, false, true, params_t&&...>
			(t, std::forward<callable_t&&>(function), width, height, x0, x1, y0, y1, std::forward<params_t&&>(params)...);
	}

	template<class callable_t, class... params_t> requires func_t<callable_t, float_t, params_t&&...>
	void get_merged_graph_texture(sf::RenderTexture& t, callable_t function, size_t width, size_t height, const float_t& x0, const float_t& x1, const float_t& y0, const float_t& y1, params_t&&... params)
	{
		return this->graph<callable_t&&, true, true, params_t&&...>
			(t, std::forward<callable_t&&>(function), width, height, x0, x1, y0, y1, std::forward<params_t&&>(params)...);
	}
};




template<class fp_t>
bool is_inf_nan(const fp_t& x)
{
	using std::isinf;
	using std::isnan;
	return isinf(x) || isnan(x);
}





_KSN_DETAIL_BEGIN

template<class fp_t, class callable_t, class... params_t>
constexpr bool newthon_method_general(callable_t func, fp_t& result, fp_t x, fp_t epsilon, size_t cycles_left, params_t&& ...params)
{
	using std::abs;

	fp_t temp, y;
	auto abs_epsilon = abs(epsilon);

	while (cycles_left)
	{
		if (is_inf_nan(x))
		{
			return false;
		}


		y = func(x, std::forward<params_t>(params)...);
		if (abs(y) <= abs_epsilon)
		{
			result = x;
			return true;
		}

		if (x == 0) x = 0.1;
		temp = x * epsilon;
		temp = (func((x + temp), std::forward<params_t>(params)...) - y) / (temp);

		if (temp == 0)
		{
			x *= (1 + epsilon * 10);
			continue;
		}


		temp = y / temp;
		x -= temp;

		if (abs(temp / x) <= abs_epsilon)
		{
			result = x;
			return true;
		}

		cycles_left--;
	}

	return false;
}

_KSN_DETAIL_END





template<class fp_t, class callable_t, class... params_t>
constexpr bool newthon_method(callable_t func, fp_t& result, fp_t x0 = 1, fp_t dx = fp_t(1e-14), size_t max_cycles = 1000, params_t&& ...params)
{
	return detail::newthon_method_general(func, result, std::move(x0), dx, max_cycles, std::forward<params_t>(params)...);
}

template<std::floating_point fp_t, class callable_t, class... params_t>
constexpr bool newthon_method(callable_t func, fp_t& result, fp_t x0 = 1, fp_t dx = std::numeric_limits<fp_t>::epsilon() * 100, size_t max_cycles = 100, params_t&& ...params)
{
	return detail::newthon_method_general(func, result, std::move(x0), dx, max_cycles, std::forward<params_t>(params)...);
}

template<std::integral int_t, class callable_t, class... params_t>
constexpr bool newthon_method(callable_t&&, int_t&, int_t = 0, int_t = 0, size_t = 0, params_t&& ...)
{
	return false;
}








_KSN_END




using ld = long double;

#define NOP() {int _ = 0; }; void()

int main()
{
	static constexpr size_t width = 800, height = 600;
	static constexpr double ratio = (long double)width / height;
	static constexpr long double x_range = 20;
	static constexpr long double x0 = 0;
	static constexpr long double x1 = x_range;
	static constexpr long double y0 = -1;
	static constexpr long double y1 = x_range / ratio - 1;

	static constexpr long double hx0 = 0.5;
	static constexpr long double hx1 = 1;

	static constexpr long double x_limit_ratio = 4.0L / 20;
	static constexpr long double tetration_base = 2.27;
	
	DBL_EPSILON;

	sf::RenderTexture t;
	t.create(width, height);

	ksn::grapher<long double> g;
	g.curve_thickness = 3;

	g.axis_enabled = true;
	g.axis_color = ksn::color_t(0x202020);
	g.axis_thickness = 3;
	//g.graph<true, false>(t, 0, width, 0, height, x0, x1, y0, y1, ksn::T1<ld>, 1.54);

	g.curve_color_real = ksn::color_t::red;
	g.custom_hx = &hx0;
	g.graph<true, false>(t, 0, size_t(800 * x_limit_ratio), 0, 600, x0, x1 * x_limit_ratio, y0, y1, ksn::T1_patched<ld>, tetration_base);

	g.axis_enabled = false;
	g.curve_color_real = ksn::color_t::blue;
	g.custom_hx = &hx1;
	g.graph<true, false>(t, 0, size_t(800 * x_limit_ratio), 0, 600, x0, x1 * x_limit_ratio, y0, y1, ksn::T1<ld>, tetration_base);

	//g.get_real_graph_texture(t, f, width, height, x0, x1, y0, y1);



	t.display();
	sf::Sprite spr_graph1(t.getTexture());



	sf::RenderWindow window(sf::VideoMode(width, height, 24), "SFML");
	window.setFramerateLimit(20);



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


			default:

				break;
			}
		}

		if (!window.isOpen()) break;

		window.clear(ksn::color_t::black);
		window.draw(spr_graph1);
		window.display();
		//window.capture().saveToFile("a.bmp");
	}

	return 0;
}




















//#include <ksn/math_formula.hpp>
//
//#include <stdio.h>
//
//#pragma comment(lib, "ksn_math.lib")
//
//int main()
//{
//	ksn::formula_ldouble f;
//	ksn::formula_ldouble::_parsed_result_t<char> operand;
//	ksn::formula_ldouble::parser_helper_t ph;
//
//	f._parse_operand("sin ( x )", operand, ph);
//	
//	
//}
