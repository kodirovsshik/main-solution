/*
#include <stdio.h>
#include <stdlib.h>
#include <new>

//#pragma comment(lib, "D:\\_need\\SFML-2.4.2x64\\lib\\sfml-system-s-d.lib")
//#pragma comment(lib, "D:\\_need\\SFML-2.4.2x64\\lib\\sfml-graphics-s-d.lib")
//#pragma comment(lib, "D:\\_need\\SFML-2.4.2x64\\lib\\sfml-window-s-d.lib")


#include <vector>
#include <cmath>

#include <ksn/math_constants.hpp>

//Returns the N roots of N'th degree real-valued polynomial (no root order)
template<class fp_t>
std::vector<fp_t> solve_polynomial(std::vector<fp_t> coeffs)
{
	for (size_t i = 0; i < coeffs.size(); ++i)
	{
		if (coeffs[coeffs.size() - i - 1] == 0)
			coeffs.pop_back();
		else
			break;
	}

	using std::sqrt;
	using std::cbrt;
	using std::cos;
	using std::acos;
	using std::pow;
	
	if (coeffs.size() <= 1) return {};
	if (coeffs.size() == 2) return { -coeffs[0] / coeffs[1] };
	if (coeffs.size() == 3)
	{
		//Quadratic
		fp_t a = std::move(coeffs[2]);
		fp_t b = std::move(coeffs[1]);
		fp_t c = std::move(coeffs[0]);
		b = b / a / 2;
		c = c / a;

		fp_t D = b * b - c;
		if (c < 0) return {};
		D = sqrt(D);
		return { -b - D, -b + D };
	}
	if (coeffs.size() == 4)
	{
		//Cubic
		fp_t a = std::move(coeffs[3]);
		fp_t b = std::move(coeffs[2]);
		fp_t c = std::move(coeffs[1]);
		fp_t d = std::move(coeffs[0]);
		a = 1 / a;
		b *= a;
		c *= a;
		d *= a;

		b /= 3;
		fp_t b1 = -b;

		a = c - 3 * b * b;
		d = b * (2 * b * b - c) + d;

		d /= 2; //q
		a /= 3; //p
		fp_t D = d * d + a * a * a;
		if (D <= 0)
		{
			//Three real roots
			d /= a;
			a = sqrt(-a);
			fp_t k = 2 * a;
			a = 1 / a;
			fp_t t = acos(d * a);
			return
			{
				fp_t(k * cos((t) / 3) + b1),
				fp_t(k * cos((t - 2 * KSN_PI) / 3) + b1),
				fp_t(k * cos((t - 4 * KSN_PI) / 3) + b1)
			};
		}
		else
		{
			d = -d;
			D = sqrt(D);
			return { fp_t(cbrt(d + D) + cbrt(d - D) + b1) };
		}
	}
	if (coeffs.size() == 5)
	{
		//Quartic
		fp_t a = 1 / std::move(coeffs[4]);
		fp_t b = a * std::move(coeffs[3]);
		fp_t c = a * std::move(coeffs[2]);
		fp_t d = a * std::move(coeffs[1]);
		fp_t e = a * std::move(coeffs[0]);

		b /= 2;

		fp_t q = d + b * (b * b - c);
		b /= 2;
		fp_t p = c - 6 * b * b;
		fp_t r = e + b * (-d + b * (c - 3 * b));

		r *= -4;
		b = (r - p * p / 3);
		p /= -3;
		a = (p * (2 * p * p - r) - q * q - 3 * r * p) * -0.5;
		c = a * a + b * b * b;
		if (c < 0)
		{
			c = -c;
			a = 2 * pow(a * a + c, 1.0 / 6) * cos(1.0 / 3 * atan2(sqrt(c), a));
		}
		else
		{
			c = sqrt(c);
			a = cbrt(a + c) + cbrt(a - c);
		}
		a -= p;
		a /= 2;
		d = b * b - r;
		if (d < 0) return {};
		d = sqrt(d);
		b = a + d;
		c = a - d;

		d = p / (c - b);
		e = c + b + 3 * p;
		e = sqrt(e);
		
		double X = 0;

		throw 1;
		//TODO: finish the formula
	}

	throw 0;
	//TODO: implement the ultimate algorithm for solving algebraic equations
}

*/

//wtf why are all the default libs are not in linked in the project settings
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "winmm.lib")

#include <ksn/window.hpp>
#include <ksn/ppvector.hpp>
#include <ksn/math_constants.hpp>
#include <ksn/stuff.hpp>

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
	return 1 / sqrtf(x);
	return _mm_rsqrt_ss(*(__m128*)&x).m128_f32[0];
}
float fsqrtf(float x)
{
	return sqrtf(x);
	return 1.f / fisqrtf(x);
}



constexpr static bool switch_bidirectional_collision_resolution_prevention = false;



struct ball
{
	//Everything in SI
	float x, y;
	float vx, vy;
	float mass, radius;
	std::unordered_set<const ball*> currently_resolving_collisions;

	float kinetic_energy() const noexcept
	{
		return this->mass / 2 * (this->vx * this->vx + this->vy * this->vy);
	}

	void draw() const noexcept
	{
		if (this->radius < 1) return;
		if (this->radius < 2)
		{
			glBegin(GL_POINTS);
			glColor3f(0, 1, 0);
			glVertex2f(this->x, this->y);
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
			glVertex2f(this->x + this->radius * cosf(β), this->y + this->radius * sinf(β));
		}

		glEnd();
	}
};



void collision_handler(ball& first, ball& second)
{
	if (first.currently_resolving_collisions.contains(&second)) return;
	first.currently_resolving_collisions.insert(&second);
	
	float v1 = hypotf(first.vx, first.vy);
	float v2 = hypotf(second.vx, second.vy);;


	//collision resolution
	{
		float dx = second.x - first.x;
		float dy = second.y - first.y;
		float distance = hypotf(dx, dy);

		float overlap = first.radius + second.radius -  distance;
		overlap *= 1.01f; //Hello from the finite precision arithmetic
		
		float multiplier = overlap / distance;
		dx *= multiplier;
		dy *= multiplier;

		float v1_prop = v1 / (v1 + v2);
		float v2_prop = 1 - v1_prop;

		first.x -= v1_prop * dx;
		first.y -= v1_prop * dy;

		second.x += v2_prop * dx;
		second.y += v2_prop * dy;
	}


	float kinetic_energy_previous = first.kinetic_energy() + second.kinetic_energy();


	__m128 angles = _mm_atan2_ps({ second.y - first.y, first.vy, second.vy, 1 }, { second.x - first.x, first.vx, second.vx, 1 });

	angles.m128_f32[1] -= angles.m128_f32[0];
	angles.m128_f32[2] -= angles.m128_f32[0];
	//[0] = collision axis's vector angle
	//[1] = first object's velocity vector angle
	//[2] = second object's velocity vector angle

	__m128 sines, cosines;
	sines = _mm_sincos_ps(&cosines, angles);
	

	float u1 = v1 * cosines.m128_f32[1]; //projection of v1 onto the collision axis
	float u2 = v2 * cosines.m128_f32[2]; //projection of v2 onto the collision axis


	{
		float M = first.mass + second.mass;
		float M1 = first.mass / M;
		float M2 = second.mass / M;
		float dmsm = M1 - M2; //mass difference over mass sum

		M1 *= 2;
		M2 *= 2;

		float temp = u1 * dmsm + M2 * u2;
		u2 = M1 * u1 - dmsm * u2;
		u1 = temp;
	}


	first.vx = u1 * cosines.m128_f32[0];
	first.vy = u1 * sines.m128_f32[0];
	second.vx = u2 * cosines.m128_f32[0];
	second.vy = u2 * sines.m128_f32[0];


	float acceleration_constant = sqrtf(kinetic_energy_previous / (first.kinetic_energy() + second.kinetic_energy()));
	
	first.vx *= acceleration_constant;
	first.vy *= acceleration_constant;
	second.vx *= acceleration_constant;
	second.vy *= acceleration_constant;

	//kinetic_energy_previous -= first.kinetic_energy() + second.kinetic_energy();
}

void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	static_assert(std::is_same_v<GLchar, char>);

	fprintf(stderr, "\ngl_error_callback() was called:\n%.*s\n", (int)length, (const char*)message);
	__debugbreak();
}

int main()
{

	//std::vector<double> coeffs = { 1, -4, 6, -4, 1}, roots;
	//roots = solve_polynomial(coeffs);

	constexpr static uint16_t win_width = 600;
	constexpr static uint16_t win_height = 600;
	constexpr static uint16_t collision_net_split_size_x = 10;
	constexpr static uint16_t collision_net_split_size_y = 10;
	constexpr static uint16_t collision_net_size_x = win_width / collision_net_split_size_x;
	constexpr static uint16_t collision_net_size_y = win_height / collision_net_split_size_y;
	constexpr static unsigned int fps_limit = 60;
	constexpr static size_t debug_log_rate = 10;

	constexpr static bool feature_collision_net = true;
	constexpr static bool feature_collision_net_draw_active = true;
	constexpr static bool feature_collision_net_draw_always = true;
	constexpr static bool feature_log_avg_fps = true;
	constexpr static bool feature_enable_debug_log = true;
	constexpr static bool feature_log_collision_resolution_preventions = false;
	constexpr static bool feature_log_kinetic_energy = true;



	bool pause = true;
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

	win.set_vsync_enabled(true);


	constexpr float root2o2 = 1.41421356f / 2;
	std::vector<struct ball> balls = //npesta would be proud //upd: i feel like i should've used a better name for this :\ 
	{
	};

	if constexpr (true)
	{
		balls.clear();
		float r = 5;
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
				b.x = x;
				b.y = y;
				b.radius = r;
				b.mass = mass_dist(engine);
				b.vx = v * cosf(a);
				b.vy = v * sinf(a);
				balls.emplace_back(std::move(b));
			}
		}
	}

	for (auto& ball : balls)
	{
		ball.currently_resolving_collisions.reserve(2);
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
		float dx = bfirst.x - bsecond.x;
		float dy = bfirst.y - bsecond.y;
		if (dx * dx + dy * dy < min_collision_distance * min_collision_distance)
		{
			collision_handler(bfirst, bsecond);
			if constexpr (_KSN_IS_DEBUG_BUILD)
			{
				float overlap = min_collision_distance - hypotf(bfirst.x - bsecond.x, bfirst.y - bsecond.y);
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
		//if (cycle_counter == 55) __debugbreak();
		//1. There used to be a section
		//upd: i already forgot myself what it was

		//2. Process current state
		if (!pause)
		{
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
					float top = ball.y - ball.radius;
					float bottom = ball.y + ball.radius;

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
						float height = ball.y - y;

						bool middle_row = false; //wheather we are currently processing a row that contains the center of a circle
						if (y < ball.y)
						{
							if (y + collision_net_split_size_y > ball.y)
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

						float left = ball.x - width;
						float right = ball.x + width;

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
				if (ball.x < ball.radius && ball.vx < 0 || ball.x > win_width - ball.radius && ball.vx > 0) { ball.vx = -ball.vx; ball.currently_resolving_collisions.clear(); }
				if (ball.y < ball.radius && ball.vy < 0 || ball.y > win_height - ball.radius && ball.vy > 0) { ball.vy = -ball.vy; ball.currently_resolving_collisions.clear(); }

				//Actual movement
				ball.x += ball.vx * dt;
				ball.y += ball.vy * dt;

				//Get rid of signed zeros
				if (ball.vx == 0) ball.vx = 0;
				if (ball.vy == 0) ball.vy = 0;

				ball.currently_resolving_collisions.clear();

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
				if constexpr (feature_log_collision_resolution_preventions)
				{
					for (const auto& ball : balls)
					{
						printf("Ball %zu collisions: [ ", &ball - &balls[0]);
						for (auto* pother : ball.currently_resolving_collisions)
						{
							printf("%zu ", pother - &balls[0]);
						}
						printf("]\n");
					}
				}
				if constexpr (feature_log_kinetic_energy)
				{
					for (const auto& ball : balls)
					{
						kinetic_energy += ball.mass / 2 * (ball.vx * ball.vx * ball.vy * ball.vy);
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