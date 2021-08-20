
#include <ksn/window.hpp>
#include <ksn/color.hpp>
#include <ksn/math_vec.hpp>
#include <ksn/math_complex.hpp>
#include <ksn/logger.hpp>
#include <ksn/time.hpp>

#include <thread>
#include <vector>
#include <semaphore>


#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_time")



class main_app_t
{
	using fp_t = double;

	struct viewport_t
	{
		ksn::vec<2, fp_t> pos;
		fp_t ratio = 0;

		static viewport_t from_center_with_ratio(fp_t center_x, fp_t center_y, fp_t ratio, size_t width, size_t height)
		{
			viewport_t result;
			result.pos[0] = center_x - ratio * width / 2;
			result.pos[1] = center_y - ratio * height / 2;
			result.ratio = ratio;
			return result;
		}

		static viewport_t from_center_with_size_x(fp_t center_x, fp_t center_y, fp_t size_x, size_t width, size_t height)
		{
			return viewport_t::from_center_with_ratio(center_x, center_y, size_x / width, width, height);
		}

		static viewport_t from_center_with_size_y(fp_t center_x, fp_t center_y, fp_t size_y, size_t width, size_t height)
		{
			return viewport_t::from_center_with_ratio(center_x, center_y, size_y / height, width, height);
		}
	} ;


	ksn::window_t window;
	ksn::file_logger logger;
	std::vector<ksn::color_bgr_t> screen1;
	std::vector<ksn::color_bgr_t> screen2;
	std::vector<ksn::color_bgr_t> *front = &screen2;
	std::vector<ksn::color_bgr_t> *back = &screen1;
	viewport_t viewport{};
	std::binary_semaphore* screen_semaphore = nullptr;
	size_t width = 0;
	size_t height = 0;
	size_t max_iterations = 20;
	uint32_t framerate_timit = 30;

	bool key_down[(int)ksn::keyboard_button_t::buttons_count]{0};
	bool key_press[(int)ksn::keyboard_button_t::buttons_count]{0};


public:
	main_app_t()
	{
		this->logger.add_file(stderr);
		this->logger.add_file("log.txt", "wb");
	}

	void init(size_t width, size_t height)
	{
		this->cleanup();
		this->screen_semaphore = new std::binary_semaphore(1);

		screen1.resize(width * height);
		screen2.resize(width * height);

		this->viewport = viewport_t::from_center_with_size_x(-0.5, 0, 4, width, height);

		this->width = width;
		this->height = height;

		memset(this->key_down, 0, sizeof(this->key_down));
	}

	int run()
	{
		if (this->width == 0)
			return -1;

		auto window_open_result = this->window.open((uint16_t)this->width, (uint16_t)(this->screen1.size() / this->width));
		if (window_open_result != ksn::window_open_result::ok)
			return window_open_result;

		try
		{
			this->mainloop();
		}
		catch (int code)
		{
			return code;
		}
		catch (...)
		{
			return -1;
		}
	}

private:

	void cleanup()
	{
		if (this->screen_semaphore)
			delete this->screen_semaphore;
		this->screen_semaphore = nullptr;

		this->width = 0;
	}

	static void _render_proc(main_app_t* app)
	{
		app->screen_semaphore->acquire();

		app->window.draw_pixels_bgr_front(app->front->data());
		app->window.tick();

		app->screen_semaphore->release();
	}

	void render()
	{
		static_assert(std::hardware_destructive_interference_size == 64);
		static_assert(sizeof(ksn::color_bgr_t) == 3);

		static std::thread threads[128];
		size_t n_threads = std::min<size_t>(128, std::thread::hardware_concurrency());
		size_t work_split_size = std::max<size_t>(this->screen1.size() / n_threads + bool(this->screen1.size() % n_threads), 22);

		size_t current_begin = 0;
		for (size_t i = 0; i < n_threads; ++i)
		{
			threads[i] = std::thread(_update_proc, this, current_begin, std::min<size_t>(current_begin + work_split_size, this->screen1.size()));
			current_begin += work_split_size;
		}

		for (size_t i = 0; i < n_threads; ++i)
		{
			if (threads[i].joinable())
				threads[i].join();
		}

		this->screen_semaphore->acquire();
		std::swap(this->front, this->back);
		this->screen_semaphore->release();

		std::thread(_render_proc, this).detach();
	}

	void update(fp_t dt)
	{
		this->poll();

		if (this->key_down[(int)ksn::keyboard_button_t::w] ||
			this->key_down[(int)ksn::keyboard_button_t::a] ||
			this->key_down[(int)ksn::keyboard_button_t::s] ||
			this->key_down[(int)ksn::keyboard_button_t::d])
		{
			const fp_t dpos = this->width / 2 * this->viewport.ratio * dt;

			fp_t vspeed = 0, hspeed = 0;
			if (this->key_down[(int)ksn::keyboard_button_t::w]) vspeed -= dpos;
			if (this->key_down[(int)ksn::keyboard_button_t::s]) vspeed += dpos;
			if (this->key_down[(int)ksn::keyboard_button_t::a]) hspeed -= dpos;
			if (this->key_down[(int)ksn::keyboard_button_t::d]) hspeed += dpos;

			this->viewport.pos[0] += vspeed;
			this->viewport.pos[1] += hspeed;
		}
		
		if (this->key_press[(int)ksn::keyboard_button_t::add] && this->max_iterations <= (SIZE_MAX / 2)) this->max_iterations *= 2;
		if (this->key_press[(int)ksn::keyboard_button_t::substract] && this->max_iterations > 1) this->max_iterations /= 2;
	}

	void scroll_handle(ksn::event_t& ev)
	{
		this->viewport.pos[0] += ev.mouse_scroll_data.x * this->viewport.ratio;
		this->viewport.pos[1] += ev.mouse_scroll_data.y * this->viewport.ratio;

		fp_t zoom_speed = fp_t(
			(this->key_down[(int)ksn::keyboard_button_t::shift_left] ||
			this->key_down[(int)ksn::keyboard_button_t::shift_right]) ?
			2 : 1.2);

		this->viewport.ratio *= std::pow(zoom_speed, -ev.mouse_scroll_data.delta);

		this->viewport.pos[0] -= ev.mouse_scroll_data.x * this->viewport.ratio;
		this->viewport.pos[1] -= ev.mouse_scroll_data.y * this->viewport.ratio;
	}

	static void _update_proc(main_app_t* app, size_t begin, size_t end)
	{
		size_t max_iterations = app->max_iterations;
		
		auto get_color = [&]
		(fp_t x0, fp_t y0) -> ksn::color_bgr_t
		{
			auto get_iterations = [&]
			() -> size_t
			{
				size_t iterations = 0;
				fp_t x = 0, y = 0, x2 = 0, y2 = 0;

				while (x2 + y2 < 4)
				{
					y = (x + x) * y + y0;
					x = x2 - y2 + x0;
					x2 = x * x;
					y2 = y * y;

					if (++iterations >= max_iterations)
						return max_iterations;
				}

				return iterations;
			};

			size_t iterations = get_iterations();

			fp_t bailout_order = (1 - fp_t(iterations - 1) / (max_iterations - 1));
			bailout_order = pow(bailout_order, 2);
			uint8_t component = (uint8_t)std::min<int>(int(bailout_order * 255 + 0.5l), 255);
			return ksn::color_bgr_t(component, component, component);
		};

		size_t x_screen = begin % app->width;

		fp_t x = app->viewport.pos[0] + app->viewport.ratio * x_screen;
		fp_t y = app->viewport.pos[1] + app->viewport.ratio * (begin / app->width);

		while (begin < end)
		{
			app->back->data()[begin] = get_color(x, y);
			if (++x_screen == app->width)
			{
				x_screen = 0;
				x = app->viewport.pos[0];
				y += app->viewport.ratio;
			}
			else
				x += app->viewport.ratio;
			++begin;
		}
	}

	void poll()
	{
		memset(this->key_press, 0, sizeof(this->key_press));

		ksn::event_t ev;
		while (this->window.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				throw 0;
				break;

			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::escape:
					throw 0;
					break;
				}

				this->key_down[(int)ev.keyboard_button_data.button] = true;
				this->key_press[(int)ev.keyboard_button_data.button] = true;
				break;

			case ksn::event_type_t::keyboard_release:
				this->key_down[(int)ev.keyboard_button_data.button] = false;
				break;

			case ksn::event_type_t::focus_lost:
				memset(this->key_down, 0, sizeof(this->key_down));
				memset(this->key_press, 0, sizeof(this->key_press));
				break;

			case ksn::event_type_t::mouse_scroll_vertical:
				scroll_handle(ev);
				break;
			}
		}
	}

	[[noreturn]] 
	void mainloop()
	{
		ksn::stopwatch renderloop_stopwatch;
		renderloop_stopwatch.start();

		//Starts window's stopwatch
		this->window.set_framerate(this->framerate_timit);

		while (true)
		{
			this->render();
			this->update(renderloop_stopwatch.restart().as_float_sec());
		}
	}


};

int main()
{

	main_app_t app;
	app.init(800, 600);
	return app.run();
}
