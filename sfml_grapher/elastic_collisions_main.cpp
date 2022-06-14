
#include <ksn/window_gl.hpp>
#include <ksn/math_constants.hpp>
#include <ksn/stuff.hpp>
#include <ksn/math_vec.hpp>
#include <ksn/color.hpp>

#define NOMINMAX
#include <Windows.h>

#include <GL/glew.h>

#include <random>


#ifdef _KSN_COMPILER_MSVC
#pragma warning(disable : 26451)

#pragma comment(lib, "libksn_time.lib")
#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_window_gl.lib")
#pragma comment(lib, "libksn_stuff.lib")

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")
#endif

#include <intrin.h>



float fisqrtf(float x)
{
	return _mm_rsqrt_ss(*(__m128*)&x).m128_f32[0];
}
float fsqrtf(float x)
{
	return 1.f / fisqrtf(x);
}



float precalculated_sin[10]; // [0; pi/2] 
//according to the profiler, it freed up 10% of CPU time

float fsinf(float x)
{
	while (x < 0) x += 2 * KSN_PIf;
	while (x > 2 * KSN_PIf) x -= 2 * KSN_PIf;

	//QuickBench™ said this is faster than having a float variable that is +/- 1
	bool negate = false;

	if (x > KSN_PIf)
	{
		x -= KSN_PIf;
		negate = true;
	}

	if (x > KSN_PIf / 2) x = KSN_PIf - x;

	x *= 20.f / KSN_PIf;
	int idx = int(x);
	if (idx == 10) return 1;

	float sin1 = precalculated_sin[idx];
	float sin2;
	if (idx == 9) _KSN_UNLIKELY
		sin2 = precalculated_sin[idx - 1];
	else _KSN_LIKELY
		sin2 = precalculated_sin[idx + 1];

	x = sin1 + (sin2 - sin1) * (x - idx);
	return negate ? -x : x;
}

float fcosf(float x)
{
	return fsinf(x + KSN_PIf / 2);
}



//wtf is that
constexpr static bool switch_bidirectional_collision_resolution_prevention = false;



struct ball
{
	ksn::vec2f pos;
	ksn::vec2f v, a;
	float mass, radius;
	ksn::color_rgb_t color;

	float kinetic_energy() const noexcept
	{
		return this->mass * this->v.abs2() / 2;
	}


	void draw() const noexcept
	{
		using fp_t = float;
		const fp_t screen_r = this->radius;
		fp_t dtheta = 1.41421f / screen_r;

		fp_t x1 = this->radius, y1 = 0;
		fp_t x_base = cosf(dtheta);
		fp_t y_base = sinf(dtheta);

		auto [x0, y0] = this->pos.data;

		glBegin(GL_POLYGON);
		glColor3ub(this->color.r, this->color.g, this->color.b);

		fp_t theta = 0;
		const fp_t max_theta = 2 * 3.1415926535f;
		while (true)
		{
			glVertex2f(x0 + x1, y0 + y1);

			theta += dtheta;
			if (theta >= max_theta)
				break;

			fp_t x2 = x1 * x_base - y1 * y_base;
			y1 = x1 * y_base + y1 * x_base;
			x1 = x2;
		};
		glEnd();
	}
};



int solve_quadratic(float a, float b, float c, float& min_root, float& max_root)
{
	a = 1 / a;
	c *= a;
	b *= 0.5f * a;
	float D = b * b - c;
	if (D < 0)
		return 0;
	D = sqrtf(D);
	min_root = -b - D;
	max_root = -b + D;
	return 2;
}

void bounce_elastic(ball& first, ball& second)
{
	ksn::vec2f t = second.pos - first.pos;

	t *= (second.v - first.v) * t / t.abs2() * 2 / (first.mass + second.mass);
	first.v += second.mass * t;
	second.v -= first.mass * t;
}

void collision_handler1(ball& first, ball& second)
{
	float v1 = first.v.abs();
	float v2 = second.v.abs();

	//collision resolution i scheme came up with
	//(that's why it works like shit)
	{
		ksn::vec2f d = second.pos - first.pos;
		float distance = d.abs();

		float overlap = first.radius + second.radius - distance;
		overlap *= 1.01f; //Hello from the finite precision arithmetic

		float multiplier = overlap / distance;
		d *= multiplier;

		float v1_prop = v1 / (v1 + v2);
		float v2_prop = 1 - v1_prop;

		first.pos -= d * v1_prop;
		second.pos += d * v2_prop;
	}

	bounce_elastic(first, second);
}
#define collision_handler(a, b, c, d) collision_handler1(a, b)
void _collision_handler(ball& first, ball& second, float dt, float dist2)
{
	//continious colision resolution

	ksn::vec2f x1 = first.pos - first.v * dt;
	ksn::vec2f x2 = second.pos - second.v * dt;
	
	ksn::vec2f dv = first.v - second.v;
	ksn::vec2f dx = x1 - x2;

	const float min_dist = first.radius + second.radius;
	const float min_dist2 = min_dist * min_dist;

	float collision_time;
	float release_time;
	if (solve_quadratic(dv.abs2(), 2* dx * dv, dx.abs2() - min_dist2, collision_time, release_time) == 0)
		__debugbreak();

	//if (collision_time < 0)
		//__debugbreak();

	//if (first.radius > 10 || second.radius > 10)
		//__debugbreak();

	first.pos = x1 + first.v * collision_time;
	second.pos = x2 + second.v * collision_time;

	bounce_elastic(first, second);

	first.pos += first.v * (dt - collision_time);
	second.pos += second.v * (dt - collision_time);
}



void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	static_assert(std::is_same_v<GLchar, char>);

	fprintf(stderr, "\ngl_error_callback() was called:\n%.*s\n", (int)length, (const char*)message);
	__debugbreak();
}



template<class T>
constexpr T rounding_up_integer_division(T a, T b)
{
	T q = a / b;
	return q + (T)bool(a % b);
	//bruh
}

int main()
{
	constexpr static uint16_t win_width = 1200;
	constexpr static uint16_t win_height = 720;
	constexpr static uint16_t collision_net_split_size_x = 7;
	constexpr static uint16_t collision_net_split_size_y = 7;
	constexpr static uint16_t collision_net_size_x = rounding_up_integer_division(win_width, collision_net_split_size_x);
	constexpr static uint16_t collision_net_size_y = rounding_up_integer_division(win_height, collision_net_split_size_y);
	constexpr static unsigned int fps_limit = 250;
	constexpr static size_t debug_log_rate = 100;

	constexpr static bool feature_collision_net = true;
	constexpr static bool feature_collision_net_draw_active = false;
	constexpr static bool feature_collision_net_draw_always = false;
	constexpr static bool feature_log_avg_fps = true;
	constexpr static bool feature_enable_debug_log = true;
	constexpr static bool feature_log_collision_resolution_preventions = false;
	constexpr static bool feature_log_kinetic_energy = false;



	bool pause = false;
	bool pause_next_frame = false;


	float dt = 1.0f / fps_limit;
	float saved_dt;


	bool keep_spawning = false;
	const int spawn_rate = 20;
	int spawn_count = 1;


	size_t cycle_counter = 0;
	size_t delay_accumulator = 0; //fps measuring


	float kinetic_energy = 0;
	float environment_resistence = 0.1f;



	ksn::window_gl_t win;
	
	ksn::window_style_t win_style{};
	win_style |= ksn::window_style::hidden;
	win_style |= ksn::window_style::border;
	win_style |= ksn::window_style::close_button;
	win_style |= ksn::window_style::caption;
	ksn::window_gl_t::context_settings context_settings = _KSN_IS_DEBUG_BUILD 
		? ksn::window_gl_t::context_settings{ 1, 1, 24, true, true }
		: ksn::window_gl_t::context_settings{};

	if (win.open(win_width, win_height, "", context_settings, win_style) != ksn::window_open_result::ok)
	{
		fprintf(stderr, "Failed to open the window\nGetLastError: %i\n", GetLastError());
		return 1;
	}

	win.context_make_current();

	printf("%s\n", glGetString(GL_VENDOR));
	printf("%s\n", glGetString(GL_RENDERER));
	printf("%s\n", glGetString(GL_VERSION));

	win.set_framerate(fps_limit);

	//if (!win.set_vsync_enabled(true))
	//{
	//	MessageBoxA(win.window_native_handle(), "Warning", "Failed to activate VSync", MB_ICONWARNING);
	//}



	for (size_t i = 0; i < 10; ++i)
		precalculated_sin[i] = sinf(i * KSN_PIf / 20.f);



	std::vector<struct ball> balls; //npesta would be proud //upd: i feel like i should've used a better name for this :\ 

	if constexpr (false)
	{
		balls.clear();
		float r = 3;
		float min_velocity = 0;
		float max_velocity = 1;
		float min_mass = 10;
		float max_mass = 100;
		
		std::normal_distribution<float> velocity_dist(min_velocity, max_velocity);
		std::uniform_real_distribution<float> mass_dist(min_mass, max_mass);
		std::uniform_real_distribution<float> angle_dist(0, 2 * KSN_PIf);
		std::mt19937_64 engine;

		for (float x = r * 1.5f; x < win_width; x += 4 * r)
		{
			for (float y = r * 1.5f; y < win_height; y += 4 * r)
			{
				float v = velocity_dist(engine);
				float a = angle_dist(engine);

				ball b;
				b.pos = ksn::vec2f{ x, y };
				b.v = ksn::vec2f{ v * fcosf(a), v * fsinf(a) };
				b.radius = r;
				b.mass = mass_dist(engine);
				balls.emplace_back(std::move(b));
			}
		}
	}
	
	if constexpr (true)
	{
		ball b;
		b.pos = { win_width / 3, win_height / 2};
		b.v = {10, 0};
		b.radius = 5;
		b.mass = 80000;
		b.color = 0xFFFF00;
		balls.push_back(b);

		b.v = {};
		b.color = 0x00FF00;
		b.mass = 625;
		b.radius = 100;
		balls.push_back(b);
	}



	std::vector<std::vector<size_t>> collision_net;
	std::vector<size_t> collision_net_nonempty_cells;
	if constexpr (feature_collision_net)
	{
		collision_net_nonempty_cells.reserve(collision_net_size_x * collision_net_size_y);
		collision_net.resize(collision_net_size_y * collision_net_size_x);

		for (auto& cell : collision_net)
		{
			cell.reserve(4);
		}
	}

	

	glClearColor(0, 0, 0, 0);
	//glClear(GL_COLOR_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, win_width, 0, win_height, -1, 1);
	
	if (glDebugMessageCallback)
		glDebugMessageCallback(gl_error_callback, nullptr);



	//win.swap_buffers();
	win.show();



	auto clock_f = std::chrono::high_resolution_clock::now;
	auto t1 = clock_f();



	auto one_frame_handler = [&]
	(bool back_in_time)
	{
		if (pause)
		{
			saved_dt = dt;
			dt = 1.0f / fps_limit;
			if (back_in_time) dt = -dt;
			pause = false;
			pause_next_frame = true;
		}
	};



	auto collision_checker = [&]
	(ball& bfirst, ball& bsecond)
	{
		float min_collision_distance = bfirst.radius + bsecond.radius;
		ksn::vec2f dist = bfirst.pos - bsecond.pos;
		float dist2 = dist.abs2();
		if (dist2 < min_collision_distance * min_collision_distance)
			collision_handler(bfirst, bsecond, dt, dist2);
	};



	auto explode_func = [&]
	(float x) -> float
	{
		return 1000 * expf(-0.03f * x) / dt;
	};

	auto explode_cell = [&]
	(int x, int y, int depth, int src_x, int src_y)
	{
		if (x < 0 || y < 0 || x >= collision_net_size_x || y >= collision_net_size_y)
			return;

		float dist = hypotf(float(x) * collision_net_split_size_x - src_x, float(y) * collision_net_split_size_y - src_y);

		for (auto id : collision_net[y * collision_net_size_x + x])
		{
			auto& ball = balls[id];
			
			auto f = (ball.pos - ksn::vec2f{src_x, src_y}).normalized();
			ball.a += explode_func(dist) * f;
		}
	};

	auto explode = [&]
	(int at_x, int at_y)
	{
		int cell_x = at_x / collision_net_split_size_x;
		int cell_y = at_y / collision_net_split_size_y;

		explode_cell(cell_x, cell_y, 0, at_x, at_y);

		float radius = 150;
		int depth = int(radius / std::min(collision_net_split_size_x, collision_net_split_size_y) + 0.5f);
		for (int i = 1; i < depth; ++i)
		{
			int x = cell_x;
			int y = cell_y + i;

			int dx = -1;
			int dy = -1;

			for (int j = 0; j < 4; ++j)
			{
				for (int k = 0; k < i; ++k)
				{
					x += dx;
					y += dy;
					explode_cell(x, y, depth, at_x, at_y);
				}
				int dx_ = -dy;
				dy = dx;
				dx = dx_;
			}
		}
	};



	while (win.is_open())
	{
		//1. There used to be a section
		//upd: i already forgot myself what it was

		if (!pause)
		{
			//2.1 spawn in new balls
			if (keep_spawning)
			{
				static int spawn_ticker = 0;
				static ksn::color_hsv_t color(0, 100, 100); //start with red
				static float hue = 0;

				if (++spawn_ticker == spawn_rate)
				{
					spawn_ticker = 0;

					if (hue > 360)
						hue -= 360;

					ball b;
					b.color = color;
					b.mass = 20;
					b.radius = 3;
					b.pos = { b.radius + 1, win_height - 5 - b.radius - 1 };
					b.v = { 200, 0 };
					for (int i = 0; i < spawn_count; ++i)
					{
						balls.push_back(b);
						hue += 0.1f;
						color.hue((uint16_t)hue);
						b.color = color;
						b.pos[1] -= 2 * b.radius + 2;
					}
				}
			}


			//2.2 Update positions
			for (auto& ball : balls)
			{
				//Screen boundary check

				const float border_thickness = 5;

				const float left_wall = border_thickness;
				const float left_bound = left_wall + ball.radius;
				if (ball.pos[0] < left_bound)
				{
					ball.pos[0] = left_bound + (left_bound - ball.pos[0]);
					if (ball.v[0] < 0)
						ball.v[0] *= -1;
				}

				const float right_wall = win_width - border_thickness;
				const float right_bound = right_wall - ball.radius;
				if (ball.pos[0] > right_bound)
				{
					ball.pos[0] = right_bound + (right_bound - ball.pos[0]);
					if (ball.v[0] > 0)
						ball.v[0] *= -1;
				}

				const float bottom_wall = border_thickness;
				const float bottom_bound = bottom_wall + ball.radius;
				if (ball.pos[1] < bottom_bound)
				{
					ball.pos[1] = bottom_bound + (bottom_bound - ball.pos[1]);
					if (ball.v[1] < 0)
						ball.v[1] *= -1;
				}

				const float top_wall = win_height - border_thickness;
				const float top_bound = top_wall - ball.radius;
				if (ball.pos[1] > top_bound)
				{
					ball.pos[1] = top_bound + (top_bound - ball.pos[1]);
					if (ball.v[1] > 0)
						ball.v[1] *= -1;
				}

				//Gravity
				const float g = 500;
				//ball.a -= ksn::vec2f{ 0.f, ball.pos[1] * g * 0.9f / win_height + 0.1f * g};
				ball.a -= ksn::vec2f{0.f, g};

				//Environment 
				float k = environment_resistence;
				ball.a -= ball.v * k;

				ball.v += ball.a * dt;

				//Actual movement
				ball.pos += ball.v * dt;

				//Reset acceleration
				ball.a = {};

				//Get rid of signed zeros
				ball.v.remove_zeros_signs();

			}


			//2.3 Process current state
			if constexpr (feature_collision_net)
			{
				//2.1. Create a collision net

				//2.1.1. Clear the previous net
				if constexpr (true)
				{
					for (auto& cell : collision_net)
					{
						cell.clear();
					}
				}

				while (true)
				{
					//2.1.2. Place new objects into the net
					for (size_t ball_index = 0; ball_index < balls.size(); ball_index++)
					{
						static auto fill_collision_net = [&]
						(size_t row, size_t column, size_t ball_id)
						{
							const size_t cell_id = row * collision_net_size_x + column;
							auto& cell = collision_net[cell_id];

							if (cell.empty())
								collision_net_nonempty_cells.push_back(cell_id);

							cell.push_back(ball_id);
						};

						const auto& ball = balls[ball_index];

						const auto corner_vector = ksn::vec2f{ 1, 1 } * ball.radius;
						auto lower = clamp(ball.pos - corner_vector, { 0, 0 }, {win_width - 1, win_height - 1});
						auto upper = clamp(ball.pos + corner_vector, { 0, 0 }, { win_width - 1, win_height - 1 });

						size_t x1 = size_t(lower[0] / collision_net_split_size_x);
						size_t x2 = size_t(upper[0] / collision_net_split_size_x);
						size_t y1 = size_t(lower[1] / collision_net_split_size_y);
						size_t y2 = size_t(upper[1] / collision_net_split_size_y);

						for (size_t i = y1; i <= y2; ++i)
							for (size_t j = x1; j <= x2; ++j)
								fill_collision_net(i, j, ball_index);
					}

					if (collision_net_nonempty_cells.empty())
						break;

					//2.2. Check for any ball collisions using a collision net and process them if there are any
					for (size_t id : collision_net_nonempty_cells)
					{
						auto& cell = collision_net[id];

						for (size_t first = 0; first < cell.size(); ++first)
						{
							for (size_t second = first + 1; second < cell.size(); ++second)
							{
								collision_checker(balls[cell[first]], balls[cell[second]]);
							}
						}

						if constexpr (false) cell.clear();
					}
					collision_net_nonempty_cells.clear();
				}
			}


			//2.4. Hanle one-frame movement
			if (pause_next_frame)
			{
				pause = true;
				pause_next_frame = false;
				dt = saved_dt;
			}
		}

		//3. Draw everything
		glClear(GL_COLOR_BUFFER_BIT);

		//glBegin(GL_TRIANGLES);	
		for (const auto& ball : balls)
		{
			//ball.place_triangles();
			ball.draw();
		}
		//glEnd();
		
		if constexpr (_KSN_IS_DEBUG_BUILD && feature_collision_net && feature_collision_net_draw_always)
		{
			glBegin(GL_LINES);
			glColor3f(0, 0, 1);

			for (int i = 0; i < win_height; i += collision_net_split_size_y)
			{
				glVertex2i(0, i);
				glVertex2i(win_width, i);
			}
			for (int j = 0; j < win_width; j += collision_net_split_size_x)
			{
				glVertex2i(j, 0);
				glVertex2i(j, win_height);
			}

			glEnd();
		}

		if constexpr (_KSN_IS_DEBUG_BUILD && feature_collision_net && feature_collision_net_draw_active)
		{
			auto drawer = [&]
			(size_t size_theshold, float r, float g, float b)
			{
				glColor3f(r, g, b);
				for (int i = 0; i < collision_net_size_y; ++i)
				{
					for (int j = 0; j < collision_net_size_x; ++j)
					{
						auto& cell = collision_net[i * collision_net_size_x + j];

						int x = j * collision_net_split_size_x;
						int y = i * collision_net_split_size_y;

						if constexpr (feature_collision_net_draw_active)
						{
							if (cell.size() >= size_theshold)
							{
								glBegin(GL_LINE_LOOP);
								glVertex2i(x, y);
								glVertex2i(x, y + collision_net_split_size_y);
								glVertex2i(x + collision_net_split_size_x, y + collision_net_split_size_y);
								glVertex2i(x + collision_net_split_size_x, y);
								glEnd();
							}
						}
					}
				}
			};

			drawer(1, 1, 0, 0);
			drawer(2, 1, 1, 0);
		}


		//4. Display new state
		win.swap_buffers();
		win.tick();

		//5. M	easure the timing
		auto t2 = clock_f();
		delay_accumulator += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();


		//5. Process the events
		ksn::event_t ev;
		while (win.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				win.close();
				break;
				
			case ksn::event_type_t::mouse_press:
				if (ev.mouse_button_data.button == ksn::mouse_button_t::left)
					explode(ev.mouse_button_data.x, win_height - 1 -  ev.mouse_button_data.y);
				break;

			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::esc:
					win.close();
					break;

				case ksn::keyboard_button_t::space:
					pause = !pause;
					break;

				case ksn::keyboard_button_t::enter:
					keep_spawning = !keep_spawning;
					break;

				case ksn::keyboard_button_t::numpad1:
					environment_resistence = 10.1f - environment_resistence;
					break;

				case ksn::keyboard_button_t::add:
					spawn_count++;
					break;

				case ksn::keyboard_button_t::substract:
					if (spawn_count)
						spawn_count--;
					break;

				case ksn::keyboard_button_t::arrow_up:
					dt = 1.0f / fps_limit;
					break;
					
				case ksn::keyboard_button_t::arrow_down:
					dt = -1.0f / fps_limit;
					break;

				case ksn::keyboard_button_t::arrow_left:
				case ksn::keyboard_button_t::arrow_right:
					one_frame_handler(ev.keyboard_button_data.button == ksn::keyboard_button_t::arrow_left);
					break;
				}
				break;

			case ksn::event_type_t::keyboard_hold:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::arrow_left:
				case ksn::keyboard_button_t::arrow_right:
					one_frame_handler(ev.keyboard_button_data.button == ksn::keyboard_button_t::arrow_left);
					break;
				}
				break;
			}
		}


		//6. Logging and some other small stuff
		t1 = clock_f();

		cycle_counter++;
		if constexpr (feature_enable_debug_log)
		{
			if ((cycle_counter % debug_log_rate) == 0)
			{
				//system("cls");
				printf("AVG FPS: %f\n", float(debug_log_rate) / delay_accumulator * 1e9);
				printf("Particles: %zu\n", balls.size()); //bruh

				if constexpr (false && feature_log_collision_resolution_preventions)
				{
					for (const auto& ball : balls)
					{
						printf("Ball %zu collisions: [ ", &ball - &balls[0]);
						//for (auto* pother : ball.currently_resolving_collisions)
						//{
							//printf("%zu ", pother - &balls[0]);
						//}
						printf("]\n");
					}
				}
				if constexpr (feature_log_kinetic_energy)
				{
					for (const auto& ball : balls)
					{
						kinetic_energy += ball.kinetic_energy();
					}
					printf("Total kinetic enetry: %g\n", kinetic_energy);
					kinetic_energy = 0;
				}
				fflush(stdout);
				delay_accumulator = 0;
			}
		}
	}


	return 0;
}