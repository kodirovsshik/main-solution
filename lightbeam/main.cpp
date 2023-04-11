
#include <ksn/window_gl.hpp>
#include <ksn/math_vec.hpp>
#include <ksn/metapr.hpp>
#include <ksn/color.hpp>

#include <GL/glew.h>

#include <format>
#include <iostream>
#include <string_view>
#include <utility>
#include <variant>

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glew32s")
#pragma comment(lib, "libksn_time")
#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_window_gl")
#pragma comment(lib, "Imm32.lib")

#define pi 3.14159265358979323


template<class Char, class Traits, class... Args>
void print(std::basic_ostream<Char, Traits>& ostr, std::basic_string_view<Char, Traits> fmt, Args&& ...args)
{
	ostr << std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
}
template<class... Args>
void print(std::string_view fmt, Args&& ...args)
{
	print(std::cout, fmt, std::forward<Args>(args)...);
}

const char* str(const void* p)
{
	if (p == 0)
		return "(null)";
	return (const char*)p;
}

template<class T>
T sqr(const T& x)
{
	return x * x;
}

ksn::vec2f reflect(ksn::vec2f x, ksn::vec2f n)
{
	return x - 2 * (x * n) * n;
}



struct particle
{
	ksn::vec2f current_position;
	ksn::vec2f velocity;

	void tick(float dt)
	{
		this->current_position += this->velocity * dt;
	}

	ksn::vec2f pos() const
	{
		return this->current_position;
	}
};

template<class T, size_t N>
struct smol_vector
{
	alignas(T) std::byte m_storage[N * sizeof(T)];
	size_t m_size = 0;

	using value_type = T;
	using iterator = T*;
	using pointer = T*;
	using const_pointer = T*;
	using reference = T&;
	using const_reference = const T&;

	constexpr smol_vector() = default;
	constexpr smol_vector(const smol_vector&) = default;
	constexpr smol_vector(smol_vector&& other) noexcept(std::is_nothrow_move_constructible_v<T>);

	constexpr smol_vector& operator=(const smol_vector&) = default;
	constexpr smol_vector& operator=(smol_vector&&) noexcept(std::is_nothrow_move_assignable_v<T>);

	constexpr ~smol_vector() noexcept(std::is_nothrow_destructible_v<T>)
	{
		while (this->m_size != 0)
			this->pop_back();
	}

	constexpr bool empty() const
	{
		return this->size() == 0;
	}
	constexpr size_t size() const
	{
		return this->m_size;
	}
	constexpr size_t capacity() const
	{
		return N;
	}

	constexpr reference operator[](size_t n) noexcept
	{
		return this->data()[n];
	}
	constexpr pointer data() noexcept
	{
		return (pointer)&this->m_storage;
	}
	constexpr const_pointer data() const noexcept
	{
		return (const_pointer)&this->m_storage;
	}

	template<class... Args>
	constexpr void emplace_back(Args&& ...args)
	{
		if (this->m_size == N)
			throw std::overflow_error("");

		auto ptr = this->data() + this->m_size;
		std::construct_at(ptr, std::forward<Args>(args)...);
		++this->m_size;
	}
	constexpr void pop_back()
	{
		if (this->m_size == 0)
			throw std::underflow_error("");
		std::destroy_at(this->data() + this->m_size - 1);
		--this->m_size;
	}

	constexpr iterator begin()
	{
		return (iterator)(&this->m_storage);
	}
	constexpr iterator end()
	{
		return (iterator)((pointer)&this->m_storage + N);
	}
};



void draw_circle(ksn::vec2f center, float radius, ksn::color_rgb_t color)
{
	glBegin(GL_POLYGON);
	glColor3ub(color.r, color.g, color.b);

	float dx = radius;
	float dy = 0;

	const float dt = 0.1f; //TODO
	const float sine = std::sin(dt), cosine = std::cos(dt);

	for (float t = 0; t < 2 * 3.141592653589793; t += dt)
	{
		glVertex2f(center[0] + dx, center[1] + dy);
		float dx1 = dx * cosine - dy * sine;
		dy = dy * cosine + dx * sine;
		dx = dx1;
	}

	glEnd();
}


struct circular_mirror
{
	ksn::vec2f position;
	float radius;
	
	float time_till_collision_with(const particle& p) const
	{
		/* BEWARE: MATH */

		const auto delta_pos = this->position - p.current_position;
		const auto& velocity = p.velocity;
		const float inv_v2 = 1 / velocity.abs2();
		const float t1 = velocity * delta_pos * inv_v2;
		//^^ Time before velocity and vector from mirror to particle become perpendicular
		//Used to solve quadratic equation
		if (t1 < 0)
			return INFINITY;

		//No idea wtf B represents
		//math moment
		const float B = (delta_pos.abs2() - this->radius * this->radius) * inv_v2;
		float D = t1 * t1 - B;
		if (D < 0)
			return INFINITY;

		return B > 0 ? t1 - sqrt(D) : t1 + sqrt(D);
	}
	void draw() const
	{
		draw_circle(this->position, this->radius, 0x808080);
	}

	ksn::vec2f normal_vector_at(ksn::vec2f pos) const
	{
		return (pos - this->position).normalized();
	}
};

using mirror_t = std::variant<circular_mirror>;

struct discrete_time
{
	float m_t = 0, m_dt;

	constexpr discrete_time(float dt) noexcept
		: m_dt(dt)
	{
	}

	template<class... Invocable>
	constexpr void step(float dt, Invocable&& ...invocable)
	{
		this->m_t += dt;
		while (this->m_t >= this->m_dt)
		{
			this->m_t -= this->m_dt;
			std::invoke(std::forward<Invocable>(invocable)...);
		}
	}
	template<class... Invocable>
	constexpr void step(Invocable&& ...invocable)
	{
		std::invoke(std::forward<Invocable>(invocable)...);
	}
};


int main()
{
	static constexpr int w = 640, h = 480;

	ksn::window_gl_t win;
	if (win.open(w, h, L"libKSN goes brrrr") != ksn::window_open_result::ok)
		return -1;
	win.context_make_current();

	print("{}\n", str(glGetString(GL_VERSION)));
	print("{}\n", str(glGetString(GL_RENDERER)));

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);

	

	float frame_dt = 1.f / 60;
	const float physics_dt = 1.f / 100;

	discrete_time frame_time(frame_dt);
	discrete_time physics_time(physics_dt);

	particle beam;
	std::vector<mirror_t> mirrors;



	auto get_soonest_collision_data = [&]
	(const particle& pt)
	{
		const mirror_t* collidee;
		float dt_till_collision = INFINITY;
		for (const auto& mirror : mirrors)
		{
			auto new_dt_till_collision = std::visit(
			[&] (auto& mirror) 
			{
				return mirror.time_till_collision_with(pt);
			}, 
			mirror
			);

			if (new_dt_till_collision >= dt_till_collision)
				continue;

			collidee = &mirror;
			dt_till_collision = new_dt_till_collision;
		}

		return std::pair{ dt_till_collision, collidee };
	};

	auto update_particle = [&]
	(particle& pt)
	{
		float particle_dt = physics_dt;

		while (particle_dt > 0)
		{
			auto [dt_till_collision, collidee] = get_soonest_collision_data(pt);
			if (dt_till_collision > particle_dt)
				break;

			pt.tick(dt_till_collision);
			particle_dt -= dt_till_collision;

			auto normal_vector = std::visit(
			[&](const auto& m)
			{
				return m.normal_vector_at(pt.pos());
			},
				*collidee
			);

			pt.velocity = reflect(pt.velocity, normal_vector);
		}

		pt.tick(particle_dt);
	};
	auto update_particles = [&]
	{
		update_particle(beam);
	};

	{
		const double start_angle = 45;
		const double velocity = 100;
		const ksn::vec2f start_pos = { 0, 0 };


		const double t = start_angle / 180 * pi;
		auto v = ksn::vec2f{ cos(t), sin(t) } * velocity;

		beam.current_position = start_pos;
		beam.velocity = v;
	}


	circular_mirror m;

	m.position = { 25, 50 };
	m.radius = 25;
	mirrors.emplace_back(m);

	m.position = { 45, 0 };
	m.radius = 5;
	mirrors.emplace_back(m);

	auto mirror_drawer = []
	(auto& mirror)
	{
		mirror.draw();
	};

	auto draw = [&]
	{
		glClear(GL_COLOR_BUFFER_BIT);

		for (auto& mirror : mirrors)
			std::visit(mirror_drawer, mirror);
		draw_circle(beam.current_position, 3, 0x00FF00);

		win.swap_buffers();
		win.tick_hybrid_sleep();
	};

	glClearColor(0, 0, 0, 0);

	win.set_framerate(60);
	while (true)
	{
		frame_time.step(draw);

		ksn::event_t ev;
		while (win.poll_event(ev))
		{
			if (ev.type == ksn::event_type_t::keyboard_press && ev.keyboard_button_data.button == ksn::keyboard_button_t::esc)
				win.close();
			if (ev.type == ksn::event_type_t::keyboard_press && ev.keyboard_button_data.button == ksn::keyboard_button_t::space)
				win.set_cursor_visible(!win.is_cursor_visible());
		}

		if (!win.is_open())
			break;

		physics_time.step(frame_dt, update_particles);
	}
}
