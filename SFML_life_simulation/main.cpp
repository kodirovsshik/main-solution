
#include <semaphore>
#include <memory_resource>



#include <ksn/window.hpp>
#include <ksn/logger.hpp>
#include <ksn/color.hpp>
#include <ksn/math_vec.hpp>

#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_time")




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
		: origin(0, 0), ratio(0), viewport_size(0, 0)
	{}

	static view_t from_center_and_minimal_size
	(ksn::vec<2, fp_t> center, ksn::vec<2, fp_t> min_size, ksn::vec<2, uint16_t> viewport)
	{
		fp_t ratio_x = viewport[0] / min_size[0];
		fp_t ratio_y = viewport[1] / min_size[1];

		view_t result;
		result.ratio = ratio_x > ratio_y ? ratio_x : ratio_y;
		result.viewport_size = viewport;
		result.origin = center - ratio / 2 * viewport;

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
};



template<class color_t, class fp_t>
class swapchain_t
{
	struct framebuffer_view
	{
		swapchain_t* p_swapchain;

		void draw_square(ksn::vec2f pos, ksn::vec2f size, color_t color, const view_t<float>& view)
		{
			ksn::vec2f square_begin = pos;
			ksn::vec2f square_end = pos + size;

			ksn::vec2f screen_begin = view.origin;
			ksn::vec2f screen_end = view.origin + view.ratio * (ksn::vec2f)view.viewport_size;

			square_begin = ksn::clamp(square_begin, screen_begin, screen_end);
			square_end = ksn::clamp(square_end, screen_begin, screen_end);


		}
	};

	friend struct framebuffer_view;

	std::vector<framebuffer_view> frame_views;
	std::vector<std::vector<color_t>> frame_buffers;
	//std::vector<std::binary_semaphore> frame_semaphores;
	std::binary_semaphore* frame_semaphores = nullptr;
	size_t frame_index = -1;
	const view_t<float>* p_view = nullptr;
	uint16_t width = 0, height = 0;


	static auto create_semaphores(size_t count)
	{
		std::binary_semaphore* arr = (std::binary_semaphore*)::operator new[](
			sizeof(std::binary_semaphore) * count, (std::align_val_t)alignof(std::binary_semaphore));

		for (size_t i = 0; i < count; ++i)
			std::construct_at(&arr[i], 1);

		return arr;
	}

	void cleanup()
	{
		delete[] this->frame_semaphores;
		this->frame_semaphores = nullptr;
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
		//for (auto& view : this->frame_views)
			//view.p_swapchain = this;

		this->frame_semaphores = create_semaphores(frames_count);

		this->p_view = view;
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

	ksn::window_t window;
	swapchain_t<ksn::color_bgr_t, fp_t> swapchain;
	view_t<fp_t> view;

	std::binary_semaphore main_loop_sema1{ 1 }, main_loop_sema2{ 0 };

	bool key_pressed[(int)ksn::keyboard_button_t::buttons_count]{ 0 };
	bool key_released[(int)ksn::keyboard_button_t::buttons_count]{ 0 };
	bool key_down[(int)ksn::keyboard_button_t::buttons_count]{ 0 };



	static constexpr uint32_t framerate_limit = 60;



	void init(uint16_t width, uint16_t height)
	{
		this->swapchain.init(width, height, &this->view);

		memset(this->key_pressed, 0, sizeof(this->key_pressed));
		memset(this->key_down, 0, sizeof(this->key_down));

		auto window_open_result = this->window.open(width, height);
		assert_throw(window_open_result == ksn::window_open_result::ok, "Failed to open the window", window_open_result);
		this->window.set_size_min_width(400);
	}
	void cleanup()
	{
	}


	static void update(main_app_t* app)
	{

	}
	static void render(main_app_t* app)
	{

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
				app->swapchain = swapchain_t<ksn::color_bgr_t, fp_t>(
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
		assert_throw(logger.add_file(stderr), "Failed to add stderr to the logger", -1);
		assert_throw(logger.add_file("log.txt", "w"), "Failed to open log.txt", -1);
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
