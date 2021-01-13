/*
This is an open source non-commercial project. Dear PVS-Studio, please check it.
PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/

#include <SFML/Graphics.hpp>

#include <windows.h>

#include <ksn/math_common.hpp>

#include <chrono>
#include <thread>

#define map_X_to_screen(width, x_start, x_end, x) (((x) - (x_start)) / ((x_end) - (x_start)) * (width))
#define map_Y_to_screen(height, y_start, y_end, y) ((height) - (((y) - (y_start)) / ((y_end) - (y_start)) * (height)))

template<class Callable, class ... Arguments>
sf::Texture get_graph_texture(uint32_t width, uint32_t height, float x1, float x2, float y1, float y2, float point_diameter, bool axis, sf::Color color, Callable&& functor, Arguments&& ...args)
{
	point_diameter /= 2;

	sf::RenderTexture result;
	result.create(width, height);

	float dx = (x2 - x1) / width;
	float dy = (y2 - y1) / height;

	if (axis)
	{
		const uint8_t axis_outline = 100;

		sf::RectangleShape axisX;

		axisX.setPosition(0, map_Y_to_screen(height, y1, y2, 0));
		axisX.setSize({ (float)width, 1.f });

		axisX.setFillColor(sf::Color::White);

		axisX.setOutlineThickness(1);
		axisX.setOutlineColor(sf::Color{ axis_outline, axis_outline, axis_outline });

		axisX.setOrigin(0, 1.5);

		result.draw(axisX);



		sf::RectangleShape axisY;

		axisY.setPosition(map_X_to_screen(width, x1, x2, 0), 0);
		axisY.setSize({ 1.f, (float)height });

		axisY.setFillColor(sf::Color::White);

		axisY.setOutlineThickness(1);
		axisY.setOutlineColor(sf::Color(axis_outline, axis_outline, axis_outline));

		axisY.setOrigin(1.5, 0);

		result.draw(axisY);


		result.display();
	}

	for (float x = x1; x <= x2; x += dx)
	{
		float y = (float)functor(x, std::forward<Arguments>(args)...);
		
		if (fpclassify(y) != FP_NORMAL)
		{
			continue;
		}
		
		sf::CircleShape point;
		//point.setPosition((x - x1) / dx, height - (y - y1) / dy);
		point.setPosition(map_X_to_screen(width, x1, x2, x), map_Y_to_screen(height, y1, y2, y));
		point.setRadius(point_diameter);
		point.setFillColor(color);
		point.setOrigin(point_diameter, point_diameter);
		
		result.draw(point);
	}

	result.display();
	return result.getTexture();
}

template<class Callable, class ... Arguments>
sf::Texture get_graph_texture_int(uint32_t width, uint32_t height, float x1, float x2, float y1, float y2, float point_diameter, bool axis, sf::Color color, Callable&& functor, Arguments&& ...args)
{
	point_diameter /= 2;

	sf::RenderTexture result;
	result.create(width, height);

	float dx = 1;
	float dy = (y2 - y1) / height;

	if (axis)
	{
		const uint8_t axis_outline = 100;

		sf::RectangleShape axis;

		axis.setFillColor(sf::Color::White);
		axis.setOutlineThickness(1);
		axis.setOutlineColor(sf::Color{ axis_outline, axis_outline, axis_outline });

		axis.setPosition(0, map_Y_to_screen(height, y1, y2, 0));
		axis.setSize({ (float)width, 1.f });
		axis.setOrigin(0, 1.5);
		result.draw(axis);

		axis.setPosition(map_X_to_screen(width, x1, x2, 0), 0);
		axis.setSize({ 1.f, (float)height });
		axis.setOrigin(1.5, 0);
		result.draw(axis);

		result.display();
	}

	sf::CircleShape point;
	point.setRadius(point_diameter);
	point.setFillColor(color);
	point.setOrigin(point_diameter, point_diameter);

	for (float x = x1; x <= x2; x += dx)
	{
		float y = (float)functor(x, std::forward<Arguments>(args)...);

		if (fpclassify(y) != FP_NORMAL)
		{
			continue;
		}

		point.setPosition(map_X_to_screen(width, x1, x2, x), map_Y_to_screen(height, y1, y2, y));

		result.draw(point);
	}

	result.display();
	return result.getTexture();
}

double f(double x)
{
	return sin(x);
}

void draw_point(float x, float y, sf::Color color, sf::RenderTarget& target, float diameter = 1)
{
	diameter /= 2;
	sf::CircleShape point;
	point.setFillColor(color);
	point.setRadius(diameter);
	point.setOrigin({ diameter, diameter });
	point.setPosition(x, y);
	target.draw(point);
}

double T(double x, uint32_t n)
{
	if (n == 0)
		return 1;
	if (n == 1)
		return x;
	return pow(x, T(x, n - 1));
}
double Tb(uint32_t n, double x)
{
	return T(x, n);
}

double Tf(double x, double n)
{
	if (
		(fpclassify(x) != FP_NORMAL && fpclassify(x) != FP_SUBNORMAL && fpclassify(x) != FP_ZERO) ||
		(fpclassify(x) != FP_NORMAL && fpclassify(x) != FP_SUBNORMAL && fpclassify(x) != FP_ZERO) ||
		n < 0
		)
	{
		return NAN;
	}

	double integral, fractional;
	fractional = modf(n, &integral);

	double current = pow(x, fractional);

	for (uint32_t i = (uint32_t)integral; i; --i)
	{
		current = pow(x, current);
	}

	return current;
}
double Tfb(double n, double x)
{
	return Tf(x, n);
}

template<class Functor, class ...Arguments>
double derivative(double x, uint8_t order, Functor&& functor, Arguments&&...parameters)
{
	if (order == 0)
	{
		return functor(x, std::forward<Arguments>(parameters)...);
	}

	double dx = 1;
	/*
	if (x == 0)
	{
		dx = 1e-8;
	}
	else
	{
		double _1 = log10(abs(x));
		int32_t _2 = int32_t(_1);
		_2 -= 10;
		_2 += order * 1;
		if (_2 & 1)
		{
			_2++;
		}
		dx = pow(10, _2);
	}*/
	double y2 = derivative(x + dx, order - 1, functor, std::forward<Arguments>(parameters)...);
	double y1 = derivative(x, order - 1, functor, std::forward<Arguments>(parameters)...);

	double dy = y2 - y1;

	return dy / dx;
}

double polynomial_evaluator(double x, double* polynomial, size_t size)
{
	double current = 0;
	while (--size != size_t(-1))
	{
		current *= x;
		current += polynomial[size];
	}

	return current;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, char*, int)
{
	const uint32_t width = 800, height = 700;
	const float x1 = 0, x2 = 8, y1 = -1, y2 = 6, diameter = 5, tower_base = 1.5;

	const size_t derivatives_count = 18;
	double polynomial[derivatives_count];
	double factorial = 1;
	for (int i = 0; i < derivatives_count; )
	{
		polynomial[i] = derivative(0, i, Tfb, tower_base) / factorial;
		factorial *= ++i;
	}

	const sf::Texture graph_texture = get_graph_texture_int(width, height, x1, x2, y1, y2, diameter, true, sf::Color::Blue, Tfb, tower_base);
	const sf::Sprite graph(graph_texture);

	const sf::Texture continuation_texture = get_graph_texture(width, height, x1, x2, y1, y2, diameter, false, sf::Color::Color(255, 255, 0, 255), polynomial_evaluator, polynomial, 9);
	const sf::Sprite continuation(continuation_texture);

	sf::RenderWindow window(sf::VideoMode(width, height), "", sf::Style::Close | sf::Style::Titlebar);
	//window.setVerticalSyncEnabled(true);

	window.draw(continuation);
	window.draw(graph);

	window.display();

	sf::Event ev;

	while (1)
	{
		bool stop = false;

		while (window.pollEvent(ev))
		{
			switch (ev.type)
			{
			case sf::Event::Closed:
				stop = true;  
				break;
			}
		}
		
		if (stop)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}