#include <stdio.h>
#include <stdlib.h>

#include <SFML/Graphics.hpp>

//#include <ksn/math.hpp>

//#pragma comment(lib, "ksn_math.lib")

//void input(const char* msg, const char *fmt, void* p)
//{
//	do
//	{
//		printf("%s", msg);
//	} while (scanf(fmt, p) != 1);
//}
//
//void worker()
//{
//	printf("y(x) = ");
//
//	char buffer[4096];
//	fgets(buffer, 4096, stdin);
//
//	ksn::formula f;
//	int parse_result;
//
//	while ((parse_result = f.parse(buffer)) != 1)
//	{
//		printf("Failed to parse: ");
//
//		if (parse_result < 1)
//		{
//			printf("Error at position %i (first character is at position 0, length = %i)", -parse_result, strlen(buffer));
//		}
//		else
//		{
//			printf("Unexpected error");
//		}
//
//		printf("\ny(x) = ");
//		fgets(buffer, 4096, stdin);
//	}
//
//	double x_start, x_end, dx, window_width, window_heigth, y_start, y_end;
//
//	input("Enter x start value: ", "%lg", &x_start);
//
//	//printf()
//}
//
//int main()
//{
//	while (1)
//	{
//		system("cls");
//		worker();
//	}
//}

#define graph_to_screen_y(y) (heigth / 2 - y * step_y)

int main()
{
	float width = 800, heigth = 600;
	
	sf::RenderTexture scratch;
	scratch.create(width, heigth);
	scratch.clear();

	sf::RenderWindow win(sf::VideoMode((int)width, (int)heigth), "");
	win.setFramerateLimit(25);

	{
		sf::RectangleShape axis_x;
		
		axis_x.setPosition(0, heigth / 2);
		axis_x.setSize({ width, 1 });
		axis_x.setOutlineColor(sf::Color::White);

		scratch.draw(axis_x);
	}

	float step_x = 10, step_y = 10;
	float x = 0;
	
	float radius = 2;

	float value = -3;
	
	while (x * step_x < width)
	{
		sf::CircleShape point;

		point.setRadius(radius / 2);
		point.setFillColor(sf::Color::White);
		
		point.setOutlineThickness(radius / 2);
		point.setOutlineColor(sf::Color::Green);

		point.setPosition(x * step_x, graph_to_screen_y(value));
		scratch.draw(point);

		x++;

		value = 0.5f * (value + 1 / value);
	}

	scratch.display();

	sf::Sprite graph;
	graph.setPosition(0, 0);
	graph.setTexture(scratch.getTexture(), true);

	sf::Event ev;
	while (win.isOpen())
	{
		win.clear();
		win.draw(graph);
		win.display();

		while (win.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
			{
				win.close();
			}
		}
	}


	return 0;
}