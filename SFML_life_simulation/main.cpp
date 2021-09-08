
#include <semaphore>
#include <memory_resource>

#include <xmmintrin.h>



#include <ksn/window.hpp>
#include <ksn/logger.hpp>
#include <ksn/color.hpp>
#include <ksn/math_vec.hpp>

#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_time")


#pragma warning(disable : 26451)




class exception_with_code
	: public std::runtime_error
{
private:
	int m_code = 0;

public:
	exception_with_code(int code, const char* what = "") noexcept
		: std::runtime_error(what), m_code(code)
	{
	}

	int err() const noexcept { return this->m_code; };
};

#define assert_throw(true_expr, msg, code) if (!(true_expr)) { throw exception_with_code(code, msg); } else ksn::nop()



class semaphore_double_sentry
{
private:
	std::binary_semaphore* s1 = nullptr, * s2 = nullptr;

public:

	semaphore_double_sentry() noexcept = default;

	semaphore_double_sentry(std::binary_semaphore* to_acquire, std::binary_semaphore* to_release) noexcept
		: s1(to_acquire), s2(to_release)
	{
		if (this->s1)
			this->s1->acquire();
	}

	semaphore_double_sentry(const semaphore_double_sentry&) noexcept = delete;
	semaphore_double_sentry(semaphore_double_sentry&& other) noexcept
	{
		std::swap(this->s1, other.s1);
		std::swap(this->s2, other.s2);
	}

	~semaphore_double_sentry()
	{
		if (this->s2)
			this->s2->release();
		this->s2 = nullptr;
	}


	semaphore_double_sentry& operator=(const semaphore_double_sentry&) noexcept = delete;
	semaphore_double_sentry& operator=(semaphore_double_sentry&& other) noexcept
	{
		std::swap(this->s1, other.s1);
		std::swap(this->s2, other.s2);
		return *this;
	}

};



template<class fp_t>
class view_t
{
	ksn::vec<2, fp_t> origin;
	mutable ksn::vec<2, fp_t> boundary;
	mutable bool boundary_cached = false;
	fp_t ratio;
	ksn::vec<2, uint16_t> viewport_size;

public:
	view_t() noexcept
		: ratio(0)
	{}

	static view_t from_center_and_minimal_size
	(ksn::vec<2, fp_t> center, ksn::vec<2, fp_t> min_size, ksn::vec<2, uint16_t> viewport)
	{
		fp_t ratio_x = viewport[0] / min_size[0];
		fp_t ratio_y = viewport[1] / min_size[1];

		view_t result;
		result.ratio = ratio_x > ratio_y ? ratio_x : ratio_y;
		result.viewport_size = viewport;
		result.origin = center - (ksn::vec<2, fp_t>)viewport / (result.ratio * 2);

		return result;
	}

	ksn::vec<2, fp_t> map_from_viewport(ksn::vec<2, uint16_t> point)
	{
		return (ksn::vec<2, fp_t>)point * this->ratio + this->origin;
	}

	void zoom_vp(fp_t factor, ksn::vec<2, uint16_t> zoom_point)
	{
		this->origin += this->map_from_viewport(zoom_point);
		this->ratio /= factor;
		this->origin -= this->map_from_viewport(zoom_point);
	}
	void zoom(fp_t factor, ksn::vec<2, fp_t> zoom_point)
	{
		this->origin += zoom_point;
		this->ratio /= factor;
		this->origin -= zoom_point * factor;
	}

	const ksn::vec2f& get_origin() const noexcept
	{
		return this->origin;
	}
	const ksn::vec2f& get_boundary() const noexcept
	{
		if (!this->boundary_cached)
		{
			this->boundary = this->origin + this->ratio * (ksn::vec<2, fp_t>)this->viewport_size;
			this->boundary_cached = true;
		}
		return this->boundary;
	}

	ksn::vec<2, int> map_to_screen(const ksn::vec2f& v) const noexcept
	{
		return (v - this->origin) * this->ratio;
	}
	int map_to_screen(float distance) const noexcept
	{
		return int(distance * this->ratio + 0.5f);
	}

	auto screen_size() const noexcept
	{
		return this->viewport_size;
	}
};


template<ksn::arithmetic T>
float fsqrtf(T x)
{
	return fsqrtf((float)x);
}
template<>
float fsqrtf<float>(float f)
{
	return 1 / _mm_rsqrt_ss(*(__m128*) & f).m128_f32[0];
}


template<class color_t>
class swapchain_t
{
public:
	struct framebuffer_view
	{
		swapchain_t* p_swapchain;

		void draw_square(ksn::vec2f pos, ksn::vec2f size, color_t color, const view_t<float>& view)
		{
			auto square_begin = view.map_to_screen(pos);
			auto square_end = view.map_to_screen(pos + size);

			square_begin = ksn::clamp<2, int>(square_begin, {}, view.screen_size());
			square_end = ksn::clamp<2, int>(square_end, {}, view.screen_size());

			int square_width = square_end[0] - square_begin[0];

			auto& screen_vector = this->p_swapchain->frame_buffers[this - &this->p_swapchain->frame_views[0]];
			auto* p_screen = screen_vector.data() + this->p_swapchain->width * square_begin[1] + square_begin[0];

			for (int i = square_begin[1]; i < square_end[1]; ++i)
			{
				std::fill(p_screen, p_screen + square_width, color);
				p_screen += this->p_swapchain->width;
			}
		}

		void draw_circle(ksn::vec2f pos, float radius, color_t color, const view_t<float>& view)
		{
			int screen_radius = view.map_to_screen(radius);
			auto screen_pos = view.map_to_screen(pos);

			auto& screen_vector = this->p_swapchain->frame_buffers[this - &this->p_swapchain->frame_views[0]];
			
			int line_start_index = std::max(0, screen_pos[1] - screen_radius);
			int line_end_index = std::min<int>(this->p_swapchain->height, screen_pos[1] + screen_radius);

			auto* p_screen = screen_vector.data() + line_start_index * (int)this->p_swapchain->width;

			for (int i = line_start_index; i <= line_end_index; ++i)
			{
				int height = screen_pos[1] - i;
				int width = int(fsqrtf(screen_radius * screen_radius - height * height) + 0.5f);
				int leftmost = std::clamp<int>(screen_pos[0] - width, 0, this->p_swapchain->width - 1);
				int rightmost = std::clamp<int>(screen_pos[0] + width, 0, this->p_swapchain->width - 1);

				std::fill(p_screen + leftmost, p_screen + rightmost + 1, color);
				p_screen += this->p_swapchain->width;
			}
		}

		void clear()
		{
			auto& screen_vector = this->p_swapchain->frame_buffers[this - &this->p_swapchain->frame_views[0]];
			memset(screen_vector.data(), 0, screen_vector.size() * sizeof(screen_vector.front()));
		}

		void present(ksn::window_t& target)
		{
			auto* p_screen = this->p_swapchain->frame_buffers[this - &this->p_swapchain->frame_views[0]].data();

			if constexpr (std::is_same_v<color_t, ksn::color_bgr_t>)
				target.draw_pixels_bgr_front(p_screen, 0, 0, this->p_swapchain->width, this->p_swapchain->height);
			else
				static_assert(false);
		}
	};

	friend struct framebuffer_view;

private:
	std::vector<framebuffer_view> frame_views;
	std::vector<std::vector<color_t>> frame_buffers;
	std::binary_semaphore* frame_semaphores = nullptr;
	size_t frame_index = -1;
	const view_t<float>* p_view = nullptr;
	uint16_t width = 0, height = 0;


	static auto create_semaphores(size_t count)
	{
		std::binary_semaphore* arr = (std::binary_semaphore*)
			//::operator new[](
			//sizeof(std::binary_semaphore) * count, (std::align_val_t)alignof(std::binary_semaphore));
			 //suddenly stopped working with msvc for some reasons saying there is a heap corruption but i'm pretty sure there is no
			malloc(sizeof(std::binary_semaphore) * count);

		if (arr == nullptr)
			throw std::bad_alloc();

		for (size_t i = 0; i < count; ++i)
			std::construct_at(&arr[i], 1);

		return arr;
	}

	void cleanup()
	{
		if (this->frame_semaphores)
		{
			std::destroy_n(this->frame_semaphores, this->frame_views.size());
			free(this->frame_semaphores);

			//delete[] this->frame_semaphores;
			this->frame_semaphores = nullptr;
		}
	}

public:
	~swapchain_t()
	{
		this->cleanup();
	}
	swapchain_t() noexcept {}
	swapchain_t(uint16_t width, uint16_t height, const view_t<float>* view, size_t frames_count = 2)
	{
		this->init(width, height, view, frames_count);
	}


	void init(uint16_t width, uint16_t height, const view_t<float>* view, size_t frames_count = 2)
	{
		this->cleanup();

		size_t frame_size = (size_t)width * height;

		this->frame_buffers.resize(frames_count);
		for (auto& frame : this->frame_buffers)
			frame.resize(frame_size);;

		this->frame_views.resize(frames_count, {this});

		this->frame_semaphores = create_semaphores(frames_count);

		this->p_view = view;

		this->width = width;
		this->height = height;
	}


	framebuffer_view& frame_acquire()
	{
		assert_throw(this->frame_buffers.size() > 0, "Empty swapchain was issued for a frame", -1);
		this->frame_index++;
		if (this->frame_index == this->frame_buffers.size())
			this->frame_index = 0;

		this->frame_semaphores[this->frame_index].acquire();
		return this->frame_views[this->frame_index];
	}

	void frame_release(framebuffer_view& framebuffer) noexcept
	{
		this->frame_semaphores[&framebuffer - &this->frame_views[0]].release();
	}

	size_t frame_count() const noexcept
	{
		return this->frame_buffers.size();
	}
};



ksn::file_logger logger;

class main_app_t
{
	using fp_t = float;

	struct particle_t
	{
	public:
		float mass = 0, radius = 0;
		ksn::color_bgr_t color;
		ksn::vec2f position, velocity, force;

		void tick(float dt) noexcept
		{
			this->velocity += dt * this->force / this->mass;
			this->position += dt * this->velocity;

			this->force = {};
		}

		void gravitate(ksn::vec2f df) noexcept
		{
			this->force += df;
		}
	};

private:
	ksn::window_t window;
	swapchain_t<ksn::color_bgr_t> swapchain;
	view_t<fp_t> view;

	std::binary_semaphore main_loop_sema1{ 1 }, main_loop_sema2{ 0 };

	bool key_pressed[(int)ksn::keyboard_button_t::buttons_count]{ 0 };
	bool key_released[(int)ksn::keyboard_button_t::buttons_count]{ 0 };
	bool key_down[(int)ksn::keyboard_button_t::buttons_count]{ 0 };

	std::vector<particle_t> particles;



	static constexpr uint32_t framerate_limit = 60;



	void init(uint16_t width, uint16_t height)
	{
		this->swapchain.init(width, height, &this->view);

		memset(this->key_pressed, 0, sizeof(this->key_pressed));
		memset(this->key_down, 0, sizeof(this->key_down));

		auto window_open_result = this->window.open(width, height);
		assert_throw(window_open_result == ksn::window_open_result::ok, "Failed to open the window", window_open_result);
		this->window.set_size_min_width(400);

		this->view = view_t<float>::from_center_and_minimal_size({}, { 100, 100 }, this->window.get_client_size());

		//this->particles.push_back({ 1, 0.5, 0xFF0000, ksn::vec2f{0, 1} });
		//this->particles.push_back({ 1, 0.5, 0x00FF00, ksn::vec2f{-KSN_ROOT3f / 2, -0.5f} });
		//this->particles.push_back({ 1, 0.5, 0x0000FF, ksn::vec2f{ KSN_ROOT3f / 2, -0.5f} });
		this->particles.push_back({ 1, 1, 0xFF0000, { -5, 5 } });
		this->particles.push_back({ 1, 1, 0xFFFF00, { 5, 5 } });
		this->particles.push_back({ 1, 1, 0x0000FF, { 5, -5 } });
		this->particles.push_back({ 1, 1, 0x00FF00, { -5, -5 } });
	}
	void cleanup()
	{
	}


	static void _gravitate(particle_t* p, size_t begin, size_t end, float total_dt)
	{
		//static constexpr float G = 6.674e-11f;
		static constexpr float G = 1;
		static constexpr float discretization_limit = 0.01f;

		if (end - begin <= 1) return;
		
		size_t mid = (begin + end) / 2;

		_gravitate(p, begin, mid, total_dt);
		_gravitate(p, mid, end, total_dt);


		float mass1 = 0, mass2 = 0;

		for (size_t i = begin; i < mid; ++i)
			mass1 += p[i].mass;

		for (size_t i = mid; i < end; ++i)
			mass2 += p[i].mass;

		while (total_dt > 0)
		{
			float dt = std::min(total_dt, discretization_limit);

			ksn::vec2f mass_center1, mass_center2;

			for (size_t i = begin; i < mid; ++i)
				mass_center1 += p[i].position * p[i].mass;

			for (size_t i = mid; i < end; ++i)
				mass_center2 += p[i].position * p[i].mass;

			mass_center1 /= mass1;
			mass_center2 /= mass2;

			ksn::vec2f distance_vector = mass_center2 - mass_center1;
			float force_abs = G * mass1 * mass2 / distance_vector.abs2();

			float angle = distance_vector.arg();
			ksn::vec2f force{ force_abs * cosf(angle), force_abs * sinf(angle) };

			for (size_t i = begin; i < mid; ++i)
				p[i].gravitate(force);

			force *= -1;

			for (size_t i = mid; i < end; ++i)
				p[i].gravitate(force);

			for (size_t i = begin; i < end; ++i)
				p[i].tick(dt);

			total_dt -= dt;
		}
	}
	static void update(main_app_t* app)
	{
		float dt = 1.f / app->window.get_framerate();

		_gravitate(app->particles.data(), 0, app->particles.size(), dt);
	}
	static void render_worker(main_app_t* app, swapchain_t<ksn::color_bgr_t>::framebuffer_view& frame)
	{
		frame.clear();

		for (const auto& particle : app->particles)
			frame.draw_circle(particle.position, particle.radius, particle.color, app->view);

		frame.present(app->window);

		app->window.tick();
		app->swapchain.frame_release(frame);
	}
	static void render(main_app_t* app)
	{
		std::thread(render_worker, app, std::ref(app->swapchain.frame_acquire())).detach();
	}
	static void poll(main_app_t* app)
	{
		memset(app->key_pressed, 0, sizeof(app->key_pressed));
		memset(app->key_released, 0, sizeof(app->key_released));

		ksn::event_t ev;
		while (app->window.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				throw exception_with_code(0, "");
				break;

			case ksn::event_type_t::keyboard_press:
				app->key_pressed[(int)ev.keyboard_button_data.button] = true;
				app->key_down[(int)ev.keyboard_button_data.button] = true;
				break;

			case ksn::event_type_t::keyboard_release:
				app->key_released[(int)ev.keyboard_button_data.button] = true;
				app->key_down[(int)ev.keyboard_button_data.button] = false;
				break;

			case ksn::event_type_t::focus_lost:
				static_assert(sizeof(app->key_down) == sizeof(app->key_released));
				memcpy(app->key_released, app->key_down, sizeof(app->key_down));
				memset(app->key_down, 0, sizeof(app->key_down));
				break;

			case ksn::event_type_t::resize:
				app->swapchain = swapchain_t<ksn::color_bgr_t>(
					ev.window_resize_data.width_new, ev.window_resize_data.width_old, &app->view, app->swapchain.frame_count());
				break;
			}
		}
	}

	[[noreturn]]
	void main_loop()
	{
		this->window.set_framerate(framerate_limit);

		while (true)
		{
			render(this);
			poll(this);
			update(this);
		}
	}



public:
	int run()
	{
		this->init(800, 600);
		try
		{
			this->main_loop();
		}
		catch (const exception_with_code& excp)
		{
			if (excp.err() == 0)
				return 0;
			throw;
		}
	}
};


int main()
{

	try
	{
		assert_throw(logger.add_file(stderr) == ksn::file_logger::add_ok, "Failed to add stderr to the logger", -1);
		assert_throw(logger.add_file("log.txt", "w") == ksn::file_logger::add_ok, "Failed to open log.txt", -1);
	}
	catch (const exception_with_code& excp)
	{
		fprintf(stderr, "Unhandled exception:\nCode = %i\nMessage:\n%s\n\n", excp.err(), excp.what());
		return excp.err();
	}
	catch (const std::exception& excp)
	{
		fprintf(stderr, "Unhandled exception:\nMessage:\n%s\n\n", excp.what());
		return -1;
	}
	catch (...)
	{
		logger.log("Unhandled exception: ???\n\n");
		return -1;
	}

	try
	{
		main_app_t app;
		return app.run();
	}
	catch (const exception_with_code& excp)
	{
		logger.log("Unhandled exception:\nCode = %i\nMessage:\n%s\n\n", excp.err(), excp.what());
		return excp.err();
	}
	catch (const std::exception& excp)
	{
		logger.log("Unhandled exception:\nMessage:\n%s\n\n", excp.what());
		return -1;
	}
	catch (...)
	{
		logger.log("Unhandled exception: ???\n\n");
		return -1;
	}

	return 0;
}
