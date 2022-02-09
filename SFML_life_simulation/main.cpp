
#include <ksn/window_gl.hpp>
#include <ksn/math_vec.hpp>
#include <ksn/time.hpp>

#include <GL/glew.h>

#include "world.hpp"
#include "config.hpp"
#include "race_semaphore.hpp"

#include <semaphore>
#include <thread>
#include <algorithm>
#include <execution>
#include <random>



#define _widen2(x) L##x
#define _widen(x) _widen2(x)
#define check(expr) _check(expr,  _widen(#expr), _widen(__FILE__), __LINE__)

void _check(bool true_expr, const wchar_t* expr, const wchar_t* file, int line)
{
	if (true_expr)
		return;

	fwprintf(stderr, L"%ws:%i: Assertion failed\n%ws\n", file, line, expr);
	_KSN_DEBUG_EXPR(__debugbreak());
	exit(1);
}


template <class Iter, class Fn>
void my_foreach(Iter begin, Iter end, Fn&& fn)
{
	static std::thread pool[32];

	const static size_t group_size = std::min<size_t>(std::size(pool), std::thread::hardware_concurrency());

	size_t global_work_size = end - begin;
	const size_t local_work_size = global_work_size / group_size + bool(global_work_size % group_size);

	size_t actual_size = group_size;
	for (size_t i = 0; i < group_size; ++i)
	{
		size_t current_size = std::min(global_work_size, local_work_size);
		if (current_size == 0)
		{
			actual_size = i;
			break;
		}
		pool[i] = std::thread(std::for_each<Iter, Fn>, begin, begin + current_size, std::ref(fn));
		begin += current_size;
		global_work_size -= current_size;
	}

	for (size_t i = 0; i < actual_size; ++i)
	{
		if (pool[i].joinable())
			pool[i].join();
	}
}


class main_app_t
{
	ksn::window_gl_t m_window;
	ksn::vec<2, uint16_t> m_window_size;
	ksn::vec2f m_world_screen_offset;
	float m_world_screen_ratio = 0;

	ksn::stopwatch m_update_stopwatch;
	float m_update_phase = 0;

	std::unique_ptr<world_t> m_world = std::make_unique<world_t>();


	bool fast = false;


	static void print_ogl_info()
	{
		printf("%s\n%s\n%s\n", glGetString(GL_VERSION), glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	}

	static void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
	{
		static_assert(std::is_same_v<GLchar, char>);

		fprintf(stderr, "\ngl_error_callback() was called:\n%.*s\n", (int)length, (const char*)message);
		__debugbreak();
	}

	static void async_renderer(main_app_t* self)
	{
		static ksn::stopwatch sw;
		printf("%lli us\n", sw.restart().as_usec());

		glFinish();
		self->m_window.swap_buffers();
		self->m_window.tick();
	}



	void set_resolution(uint16_t width, uint16_t height)
	{
		ksn::window_gl_t::context_settings context_settings{};
		context_settings.ogl_version_major = 4;
		context_settings.ogl_version_minor = 3;
		context_settings.ogl_debug = true;

		ksn::window_style_t style = 0;
		style |= ksn::window_style::border;
		style |= ksn::window_style::caption;
		style |= ksn::window_style::close_min_max;
		style |= ksn::window_style::hidden;

		this->m_window_size = { width, height };
		check(this->m_window.open(width, height, L"ඞ", context_settings, style) == ksn::window_open_result::ok);

		this->m_window.context_make_current();

		if (glDebugMessageCallback)
			glDebugMessageCallback(gl_error_callback, nullptr);

		glViewport(0, 0, width, height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width, 0, height, -1, 1);
	}

	void main_loop()
	{
		this->m_window.show();

		while (true)
		{
			this->render();
			this->update();
			if (!this->m_window.is_open())
				return;
		}
	}
	void poll()
	{
		ksn::event_t ev;
		while (this->m_window.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				this->m_window.close();
				return;

			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::numpad0:
					this->fast = !this->fast;
					break;
				}
				break;
			}
		}
	}
	void update()
	{
		this->poll();

		if (fast || true)
		{
			this->tick();
		}
		else
		{
			this->m_update_phase += this->m_update_stopwatch.restart().as_float_sec();
			//printf("%f\n", this->m_update_phase);

			while (this->m_update_phase >= config::update_period)
			{
				this->m_update_phase -= config::update_period;
				this->tick();
				//this->m_update_phase = fmodf(this->m_update_phase, config::update_period);
			}
		}
	}
	void tick()
	{
		//std::for_each(std::execution::par_unseq, (world_entry*)this->m_world->world, (world_entry*)std::end(this->m_world->world), [=]
		my_foreach((world_entry*)this->m_world->world, (world_entry*)std::end(this->m_world->world), [=]
		(world_entry& entry)
		{
			entry.sema.acquire_for_write();
			if (entry.type != object_type_stranger)
			{
				entry.sema.release_from_write();
				return;
			}
			entry.p_creature->tick(this->m_world.get());
			entry.sema.release_from_write();
		});
	}

	void render()
	{
		if (this->fast)
		{
			static int64_t frame_counter = 0;
			if ((++frame_counter % 75) != 0)
				return;
		}
		else
			fwrite("a", 1, 1, stdout);

		glFinish();
		this->m_window.swap_buffers();

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_QUADS);

		for (const auto& column : this->m_world->world)
			for (const auto& elem : column)
			{
				if (elem.type == object_type_stranger)
				{
					glColor3ub(elem.p_creature->color[0], elem.p_creature->color[1], elem.p_creature->color[2]);
					auto pos = (ksn::vec2f)(elem.p_creature->pos * (int)config::cell_size);

					glVertex2f(pos[0], pos[1]);
					glVertex2f(pos[0] + config::cell_size, pos[1]);
					glVertex2f(pos[0] + config::cell_size, pos[1] + config::cell_size);
					glVertex2f(pos[0], pos[1] + config::cell_size);
				}
				else if (elem.energy != 0)
				{
					glColor3ub(config::food_color[0], config::food_color[1], config::food_color[2]);
					auto pos = (ksn::vec2f)(m_world->pos(elem) * (int)config::cell_size);

					glVertex2f(pos[0], pos[1]);
					glVertex2f(pos[0] + config::cell_size, pos[1]);
					glVertex2f(pos[0] + config::cell_size, pos[1] + config::cell_size);
					glVertex2f(pos[0], pos[1] + config::cell_size);
				}
			}

		glEnd();

		glBegin(GL_LINES);
		glColor3f(1, 1, 1);
		for (int i = 0; i < this->m_world->width; ++i)
		{
			glVertex2f(i * config::cell_size, 0);
			glVertex2f(i * config::cell_size, this->m_world->height * config::cell_size);
		}
		for (int i = 0; i < this->m_world->height; ++i)
		{
			glVertex2f(0, i * config::cell_size);
			glVertex2f(this->m_world->width * config::cell_size, i * config::cell_size);
		}
		glEnd();

		glFlush();

		if (!this->fast)
			this->m_window.tick();
	}

	void init()
	{
		this->set_resolution(800, 600);
		this->m_window.set_framerate(60);
		this->print_ogl_info();

		srand((unsigned)time(0));
	}
	void game_init()
	{
		static constexpr size_t n_creatures = 500;
		std::minstd_rand rand;
		for (size_t i = 0; i < n_creatures; ++i)
		{
			while (true)
			{
				ksn::vec2i pos = { rand() % m_world->width, rand() % m_world->height };
				world_entry* p = this->m_world->at(pos);
				if (p->type)
					continue;

				this->m_world->place_creature_create(pos);
				break;
			}
		}
		for (size_t i = 0; i < n_creatures; ++i)
		{
			while (true)
			{
				ksn::vec2i pos = { rand() % m_world->width, rand() % m_world->height };
				this->m_world->at(pos)->energy += rand() / (float)RAND_MAX * 20;
				break;
			}
		}
		this->m_update_stopwatch.start();
	}
public:
	void run()
	{
		this->init();
		this->game_init();
		this->main_loop();
	}
};

int main()
{
	main_app_t app;
	app.run();
	return 0;
}