
#include <ksn/window.hpp>
//#include <ksn/ppvector.hpp>
#include <ksn/math_constants.hpp>
#include <ksn/stuff.hpp>
#include <ksn/math_vec.hpp>

#define NOMINMAX
#include <Windows.h>

#include <GL/glew.h>

#include <chrono>
#include <unordered_set>
#include <random>


#ifdef _KSN_COMPILER_MSVC
#pragma warning(disable : 26451)
#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_x86_instruction_set.lib")
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
//which is now twice less than when it had to compute sine every time

float fsinf(float x)
{
	while (x < 0) x += 2 * KSN_PIf;
	while (x > 2 * KSN_PIf) x -= 2 * KSN_PIf;

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



constexpr static bool switch_bidirectional_collision_resolution_prevention = false;



struct ball
{
	//Everything in SI
	ksn::vec2f pos;
	ksn::vec2f v;
	float mass, radius;

	float kinetic_energy() const noexcept
	{
		return this->mass * this->v.abs2() / 2;
	}

	void draw() const noexcept
	{
		if (this->radius < 1) return;
		if (this->radius < 2)
		{
			glBegin(GL_POINTS);
			glColor3f(0, 1, 0);
			glVertex2f(this->pos[0], this->pos[1]);
			glEnd();
			return;
		}

		glBegin(GL_POLYGON);
		glColor3f(0, 1, 0);

		float inv_radius = 1.f / this->radius;
		float β;
		float dβ = inv_radius * (1 + inv_radius * inv_radius / 6);
		//^^^ First two terms of arcsin(x) expansion ^^^

		for (β = 0; β < 2 * KSN_PI; β += dβ)
		{
			glVertex2f(this->pos[0] + this->radius * fcosf(β), this->pos[1] + this->radius * fsinf(β));
		}

		glEnd();
	}
};



void collision_handler(ball& first, ball& second)
{
	float v1 = first.v.abs();
	float v2 = second.v.abs();


	//collision resolution
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


	ksn::vec2f t = second.pos - first.pos;

	t *= (second.v - first.v) * t / t.abs2() * 2 / (first.mass + second.mass);
	first.v += second.mass * t;
	second.v -= first.mass * t;
}



void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	static_assert(std::is_same_v<GLchar, char>);

	fprintf(stderr, "\ngl_error_callback() was called:\n%.*s\n", (int)length, (const char*)message);
	__debugbreak();
}



int main()
{

	constexpr static uint16_t win_width = 600;
	constexpr static uint16_t win_height = 600;
	constexpr static uint16_t collision_net_split_size_x = 10;
	constexpr static uint16_t collision_net_split_size_y = 10;
	constexpr static uint16_t collision_net_size_x = win_width / collision_net_split_size_x;
	constexpr static uint16_t collision_net_size_y = win_height / collision_net_split_size_y;
	constexpr static unsigned int fps_limit = 60;
	constexpr static size_t debug_log_rate = 10;

	constexpr static bool feature_collision_net = false;
	constexpr static bool feature_collision_net_draw_active = true;
	constexpr static bool feature_collision_net_draw_always = true;
	constexpr static bool feature_log_avg_fps = true;
	constexpr static bool feature_enable_debug_log = true;
	constexpr static bool feature_log_collision_resolution_preventions = false;
	constexpr static bool feature_log_kinetic_energy = true;



	bool pause = false;
	bool pause_next_frame = false;



	float dt = 1.0f / fps_limit;
	float saved_dt;



	ksn::window_t win;
	
	ksn::window_t::style_t win_style = ksn::window_t::style::hidden | ksn::window_t::style::default_style;
	ksn::window_t::context_settings context_settings = _KSN_IS_DEBUG_BUILD 
		? ksn::window_t::context_settings{ 1, 1, 24, true, true }
		: ksn::window_t::context_settings{};

	if (win.open(win_width, win_height, "", context_settings, win_style) != ksn::window_t::error::ok)
	{
		fprintf(stderr, "Failed to open the window\nGetLastError: %i\n", GetLastError());
		return 1;
	}

	win.make_current();

	printf("%s\n", glGetString(GL_VENDOR));
	printf("%s\n", glGetString(GL_RENDERER));
	printf("%s\n", glGetString(GL_VERSION));

	if (!win.set_vsync_enabled(true))
	{
		MessageBoxA(win.window_native_handle(), "Warning", "Failed to activate VSync", MB_ICONWARNING);
	}


	for (size_t i = 0; i < 10; ++i)
	{
		precalculated_sin[i] = sinf(i * KSN_PIf / 20.f);
	}


	std::vector<struct ball> balls = //npesta would be proud //upd: i feel like i should've used a better name for this :\ 
	{
	};

	if constexpr (true)
	{
		balls.clear();
		float r = 10;
		float min_velocity = 0;
		float max_velocity = 100;
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
	


	std::vector<size_t> collision_net[win_height / collision_net_split_size_y][win_width / collision_net_split_size_x];
	if (feature_collision_net)
	{
		for (auto& row : collision_net)
		{
			for (auto& cell : row)
			{
				cell.reserve(3);
			}
		}
	}

	

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glOrtho(0, win_width, 0, win_height, -1, 1);
	
	glDebugMessageCallback(gl_error_callback, nullptr);



	win.swap_buffers();
	win.show();



	size_t cycle_counter = 0;
	size_t delay_accumulator = 0;



	float kinetic_energy = 0;



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
		if (dist.abs2() < min_collision_distance * min_collision_distance)
		{
			collision_handler(bfirst, bsecond);
			if constexpr (_KSN_IS_DEBUG_BUILD)
			{
				float overlap = min_collision_distance - dist.abs();
				if (overlap > 1000)
				{
					char buff[64];
					sprintf_s(buff, 64, "Tick: %zu, overlap of %g", cycle_counter, overlap);
					MessageBoxA(win.window_native_handle(), buff, "Overlap", MB_ICONERROR);
					pause = true;
				}
			}
		}
	};



	while (win.is_open())
	{
		//1. There used to be a section
		//upd: i already forgot myself what it was

		if (!pause)
		{
			//2. Process current state
			if (feature_collision_net)
			{
				//2.1. Create a collision net

				//2.1.1. Clear the previous net
				if constexpr (_KSN_IS_DEBUG_BUILD) //on release cleared just after collision processing
				{
					for (auto& row : collision_net)
					{
						for (auto& cell : row)
						{
							cell.clear();
						}
					}
				}

				//2.1.2. Place new objects into the net
				for (size_t ball_index = 0; ball_index < balls.size(); ball_index++)
				{
					const auto& ball = balls[ball_index];
					float top = ball.pos[1] - ball.radius;
					float bottom = ball.pos[1] + ball.radius;

					top -= fmodf(top, collision_net_split_size_y);
					if (top < 0)
						top = 0;

					//bottom += (collision_net_split_size_y - fmodf(bottom, collision_net_split_size_y));
					if (bottom > win_height - 1)
						bottom = win_height - 1;

					size_t y_index = size_t(top / collision_net_split_size_y + 0.5f);
					for (float y = top; y < bottom; y += collision_net_split_size_y)
					{
						float width;
						float height = ball.pos[1] - y;

						bool middle_row = false; //wheather we are currently processing a row that contains the center of a circle
						if (y < ball.pos[1])
						{
							if (y + collision_net_split_size_y > ball.pos[1])
								middle_row = true;
							else
								height -= collision_net_split_size_y;
						}

						if (middle_row)
						{
							width = ball.radius;
						}
						else
						{
							float width2 = ball.radius * ball.radius - height * height;
							width = width2 > 0 ? fsqrtf(width2) : 0;
						}

						float left = ball.pos[0] - width;
						float right = ball.pos[0] + width;

						left -= fmodf(left, collision_net_split_size_x);
						if (left < 0)
							left = 0;
						right += (collision_net_split_size_x - fmodf(right, collision_net_split_size_x));
						if (right > win_width)
							right = win_width;

						size_t x_index = size_t(left / collision_net_split_size_x + 0.5f);
						size_t x_last_index = size_t(right / collision_net_split_size_y + 0.5f);

						if (y == top || y + collision_net_split_size_y > bottom)
						{
							for (size_t i = x_index; i < x_last_index; ++i)
							{
								if (i < collision_net_size_x)
									collision_net[y_index][i].push_back(ball_index);
								else
									break;
							}
						}
						else
						{
							x_last_index -= 1;
							if (x_index < 0) x_index = 0;
							if (x_last_index >= collision_net_size_x) x_last_index = collision_net_size_x - 1;

							collision_net[y_index][x_index].push_back(ball_index);
							if (x_index != x_last_index)
								collision_net[y_index][x_last_index].push_back(ball_index);
						}

						y_index++;
					}
				}

				//2.2. Check for any ball collisions using a collision net and process them if there are any
				for (auto& row : collision_net)
				{
					for (auto& cell : row)
					{
						for (size_t first = 0; first < cell.size(); ++first)
						{
							for (size_t second = first + 1; second < cell.size(); ++second)
							{
								collision_checker(balls[cell[first]], balls[cell[second]]);
							}
						}

						if constexpr (!_KSN_IS_DEBUG_BUILD) cell.clear();
					}
				}
			}
			else
			{
				;
				//Check for collisions by each-with-each principle
				for (size_t first = 0; first < balls.size(); ++first)
				{
					for (size_t second = first + 1; second < balls.size(); ++second)
					{
						collision_checker(balls[first], balls[second]);
					}
				}
			}


			//2.3 Update positions
			for (auto& ball : balls)
			{
				//Screen boundary check
				if (ball.pos[0] < ball.radius && ball.v[0] < 0 || ball.pos[0] > win_width - ball.radius && ball.v[0] > 0) { ball.v[0] = -ball.v[0]; /*ball.currently_resolving_collisions.clear();*/ }
				if (ball.pos[1] < ball.radius && ball.v[1] < 0 || ball.pos[1] > win_height - ball.radius && ball.v[1] > 0) { ball.v[1] = -ball.v[1]; /*ball.currently_resolving_collisions.clear()*/; }

				//Actual movement
				ball.pos += ball.v * dt;

				//Get rid of signed zeros
				ball.v.remove_zeros_signs();

				//ball.currently_resolving_collisions.clear();

				////Clear resolved collisions
				//std::erase_if(ball.currently_resolving_collisions, [&](const struct ball* p)
				//	{
				//		const auto& other_ball = *p;
				//		float min_collision_distance = ball.radius + other_ball.radius;
				//		float dx = ball.x - other_ball.x;
				//		float dy = ball.y - other_ball.y;
				//		return dx * dx + dy * dy >= min_collision_distance * min_collision_distance;
				//	});
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

		for (const auto& ball : balls)
		{
			ball.draw();
		}
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
						auto& cell = collision_net[i][j];

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


		//5. Measure the timing
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

			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::esc:
					win.close();
					break;

				case ksn::keyboard_button_t::space:
					pause = !pause;
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