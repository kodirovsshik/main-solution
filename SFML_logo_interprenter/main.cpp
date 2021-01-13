#include <SFML/Graphics.hpp>

struct slider
{
	double g_x, g_y;
	double g_x_min, g_x_max;
	double g_y_min, g_y_max;
	double g_value_min, g_value_max;

	bool g_is_active;

	sf::Drawable* g_p_drawable = nullptr;
	sf::Drawable* g_p_drawable_on_select = nullptr;

	slider()
	{
		memset(this, 0, sizeof(slider));
	}
	/*slider(const slider& other)
	{
		memcpy(this, &other, sizeof(slider));
	}
	slider& operator=(const slider& other)
	{
		this->slider::slider(other);
		return *this;
	}*/

	void render(sf::RenderTarget& target) const
	{
		sf::Drawable* p_drawable = this->g_is_active ? this->g_p_drawable_on_select : this->g_p_drawable;
		if (p_drawable)
		{
			sf::Transformable* p_transformable = dynamic_cast<sf::Transformable*>(p_drawable);
			if (p_transformable)
			{
				p_transformable->setPosition((float)g_x, (float)g_y);
			}

			target.draw(*p_drawable);
		}
	}

	void slide()
	{
		if (this->g_x < this->g_x_min)
		{
			this->g_x = this->g_x_min;
		}
		if (this->g_x > this->g_x_max)
		{
			this->g_x = this->g_x_max;
		}
		
		if (this->g_y < this->g_y_min)
		{
			this->g_y = this->g_y_min;
		}
		if (this->g_y > this->g_y_max)
		{
			this->g_y = this->g_y_max;
		}
	}
	void slide(double x, double y)
	{
		if (x < this->g_x_min)
		{
			x = this->g_x_min;
		}
		if (x > this->g_x_max)
		{
			x = this->g_x_max;
		}

		if (y < this->g_y_min)
		{
			y = this->g_y_min;
		}
		if (y > this->g_y_max)
		{
			y = this->g_y_max;
		}

		this->g_x = x;
		this->g_y = y;
	}

	double operator()() const
	{
		double dx = this->g_x_max - this->g_x_min;
		double dy = this->g_y_max - this->g_y_min;
		double dv = this->g_value_max - this->g_value_min;

		double len;

		if (dx == 0 && dy != 0)
		{
			len = (this->g_y - this->g_y_min) / dy;
		}
		else if (dx != 0 && dy == 0)
		{
			len = (this->g_x - this->g_x_min) / dx;
		}
		else
		{
			dx = (this->g_x - this->g_x_min) / dx;
			dy = (this->g_y - this->g_y_min) / dy;
			len = sqrt(dx * dx + dy * dy);
		}

		return this->g_value_min + dv * len;
	}
};

bool isInCircle(double x, double y, double x0, double y0, double r)
{
	double dx = x - x0;
	double dy = y - y0;

	dx *= dx;
	dy *= dy;

	return dx + dy <= r * r;
}

int main()
{
	sf::CircleShape cs1;
	cs1.setFillColor(sf::Color(150, 150, 150));
	cs1.setOrigin(15, 15);
	cs1.setRadius(15);

	sf::CircleShape cs1s;
	cs1s.setFillColor(sf::Color(100, 100, 100));
	cs1s.setOrigin(15, 15);
	cs1s.setRadius(15);

	slider obj_s1;
	obj_s1.g_x = obj_s1.g_x_min = 100;
	obj_s1.g_x_max = 500;
	obj_s1.g_y_min = obj_s1.g_y_max = 100;
	obj_s1.slide();
	obj_s1.g_value_min = 60;
	obj_s1.g_value_max = 120;
	obj_s1.g_p_drawable = &cs1;
	obj_s1.g_p_drawable_on_select = &cs1s;

	sf::RenderWindow win;
	win.create(sf::VideoMode(600, 600), "Sliders");

	while (win.isOpen())
	{
		sf::Event ev;
		while (win.pollEvent(ev))
		{
			switch (ev.type)
			{
			case sf::Event::Closed:
				win.close();
				break;

			case sf::Event::MouseButtonPressed:
				if (ev.mouseButton.button == sf::Mouse::Left)
				{
					if (isInCircle(ev.mouseButton.x, ev.mouseButton.y, obj_s1.g_x, obj_s1.g_y, 15))
					{
						obj_s1.g_is_active = true;
					}
				}
				break;

			case sf::Event::MouseButtonReleased:
				obj_s1.g_is_active = false;
				break;

			case sf::Event::MouseMoved:
				if (obj_s1.g_is_active)
				{
					obj_s1.slide(ev.mouseMove.x, ev.mouseMove.y);
				}
				break;
			}
		}

		if (win.isOpen() == false)
		{
			break;
		}

		win.clear();

		obj_s1.render(win);

		win.setFramerateLimit((int)(obj_s1() + .5));
		win.display();
	}

	return 0;
}