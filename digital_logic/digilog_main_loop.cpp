
#include "opencl.hpp"
#include "graphics.hpp"
#include "window.hpp"

#include <memory>

#include <ksn/time.hpp>
#include <ksn/math_matrix.hpp>




#pragma warning(disable : 4996)





bool key_pressed[(size_t)ksn::keyboard_button_t::buttons_count] = { 0 };
bool key_held[(size_t)ksn::keyboard_button_t::buttons_count] = { 0 };
bool key_repeat[(size_t)ksn::keyboard_button_t::buttons_count] = { 0 };


void test_callback(object_t& test, float dt)
{
	float da = 0;
	float degrees_per_sec = -90;

	ksn::vec2f dpos;
	float speed = key_held[(size_t)ksn::keyboard_button_t::shift_left] ? 150.f : 30.f;

	if (key_held[(size_t)ksn::keyboard_button_t::w])
		dpos += ksn::vec2f{ 0, -1 };
	if (key_held[(size_t)ksn::keyboard_button_t::s])
		dpos += ksn::vec2f{ 0, +1 };

	if (dpos[1] == 0)
		degrees_per_sec = 0;
	if (dpos[1] < 0)
		degrees_per_sec = -degrees_per_sec;

	if (key_held[(size_t)ksn::keyboard_button_t::a])
		da -= degrees_per_sec * dt;
	if (key_held[(size_t)ksn::keyboard_button_t::d])
		da += degrees_per_sec * dt;

	test.rotate_degrees(da);

	if (dpos != ksn::vec2f{})
	{
		dpos.normalize();

		float a = test.get_rotation();
		ksn::matrix<2, 2> rotator{ cosf(a), -sinf(a), sinf(a), cosf(a) };

		test.m_transform_data.m_position += rotator * dpos * speed * dt;
	}
}





struct
{
	std::vector<std::unique_ptr<object_t>> objects;
} scene;

bool digilog_update(float dt)
{
	for (const auto& p : scene.objects)
	{
		if (p->p_update_callback)
			p->p_update_callback(*p, dt);
	}

	return true;
}

ksn::window_t window_unscaled;

void digilog_render()
{
	draw_adapter.clear({});
	
	for (const auto& p : scene.objects)
	{
		draw_adapter.draw(*p);
	}

	static size_t datasize = sizeof(ksn::color_bgra_t) * window.size.first * window.size.second * draw_adapter.m_scaling * draw_adapter.m_scaling;

	if (draw_adapter.m_scaling > 1)
	{
		static ksn::color_bgra_t* unscaled_data = (ksn::color_bgra_t*)malloc(datasize);
		//cl_data.q.enqueueReadBuffer(draw_adapter.m_screen_videodata, CL_TRUE, 0, datasize, unscaled_data);
		//window_unscaled.draw_pixels_bgra_front(unscaled_data);
		//free(unscaled_data);
	}

	draw_adapter.display();
	//window.window.tick();
	window.window.tick_hybrid_sleep();
}



int digilog_main_loop()
{

	static constexpr float fps_update_period = 0.5f;

	uint8_t scaling_factor = 4;
	draw_adapter.set_image_scaling(scaling_factor);

	const uint32_t framerate_limit = 60;

	if (framerate_limit)
		window.window.set_framerate(framerate_limit);


	//window_unscaled.open(window.size.first * scaling_factor, window.size.second * scaling_factor, "", ksn::window_style::border | ksn::window_style::close_button);


	bool stop = false;
	
	ksn::stopwatch sw;
	//ksn::stopwatch event_sw;
	int64_t leap_nanodt;
	float cycle_dt = 0, dt = 1.f / (framerate_limit ? framerate_limit : window.window.get_monitor_framerate());

	uint64_t tick_counter = 0;
	uint64_t tick_fps_counter = 0;



	texture_t txt_test;
	txt_test.load("test.png");

	//object_t& obj_test = *scene.objects.emplace_back(std::make_unique<object_t>());

	//obj_test.set_sprite(&txt_test, { 25, 25 }, { 50, 50 });
	//obj_test.m_transform_data.m_position = { 25, 25 };
	//obj_test.m_transform_data.m_rotation_data = { 0, 1 };
	//obj_test.m_transform_data.m_rotation_origin = { 25, 25 };

	//obj_test.p_update_callback = test_callback;
	//obj_test.set_rotation_degrees(10);


	while (true)
	{
		//Update

		if (!digilog_update(dt))
			break;





		//Render

		digilog_render();





		//Poll and process events

		for (size_t i = 0; i < ksn::countof(key_pressed); ++i) key_held[i] |= key_pressed[i];

		memset(key_pressed, 0, sizeof(key_pressed));



		//window_unscaled.discard_all_events();

		ksn::event_t ev;
		while (window.window.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::keyboard_press:
				key_pressed[(size_t)ev.keyboard_button_data.button] = true;
				break;

			case ksn::event_type_t::keyboard_hold:
				key_repeat[(size_t)ev.keyboard_button_data.button] = true;
				break;

			case ksn::event_type_t::keyboard_release:
				key_pressed[(size_t)ev.keyboard_button_data.button] = false;
				key_held[(size_t)ev.keyboard_button_data.button] = false;
				break;

			case ksn::event_type_t::resize:
				draw_adapter.resize(ev.window_resize_data.width_new, ev.window_resize_data.height_new);
				break;

			case ksn::event_type_t::close:
				stop = true;
				break;

			case ksn::event_type_t::focus_lost:
				memset(key_pressed, 0, sizeof(key_pressed));
				memset(key_held, 0, sizeof(key_held));
			}

		}

		if (stop) break;

		//cycle_dt -= event_sw.stop().as_nsec() / 1e9f;





		//Aux

		leap_nanodt = sw.restart();
		cycle_dt += (dt = leap_nanodt / 1e9f);

		if (cycle_dt > fps_update_period)
		{
			char buffer[128];
			sprintf(buffer, "Digital Logic Emulator by kodirovsshik - %i FPS", int(tick_fps_counter / cycle_dt));
			window.window.set_title(buffer);

			cycle_dt = 0;
			tick_fps_counter = 0;
		}
		else
			++tick_fps_counter;

		++tick_counter;
	}



	return 0;
}