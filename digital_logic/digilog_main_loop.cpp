
#include "opencl.hpp"
#include "graphics.hpp"
#include "window.hpp"

#include <ksn/time.hpp>




#pragma warning(disable : 4996)





static bool key_pressed[(size_t)ksn::keyboard_button_t::buttons_count] = { 0 };
static bool key_held[(size_t)ksn::keyboard_button_t::buttons_count] = { 0 };
static bool key_repeat[(size_t)ksn::keyboard_button_t::buttons_count] = { 0 };





bool digilog_update(float dt)
{


	return true;
}

void digilog_render()
{
	draw_adapter.clear({});

	

	draw_adapter.display(window.window);
	window.window.tick();
}



int digilog_main_loop()
{

	static constexpr uint8_t fps_freq_multiplier = 2;

	window.window.set_framerate(60);

	bool stop = false;
	
	ksn::stopwatch sw;
	ksn::stopwatch event_sw;
	int64_t leap_nanodt;
	float cycle_fdt = 0;

	uint64_t tick_counter = 0;

	while (!stop)
	{
		//Render
		digilog_render();




		//Poll and process events

		memset(key_pressed, 0, sizeof(key_pressed));


		event_sw.start();

		ksn::event_t ev;
		while (window.window.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				break;

			case ksn::event_type_t::keyboard_press:
				key_pressed[(size_t)ev.keyboard_button_data.button] = true;
				break;

			case ksn::event_type_t::keyboard_hold:
				key_repeat[(size_t)ev.keyboard_button_data.button] = true;
				break;

			case ksn::event_type_t::keyboard_release:
				key_pressed[(size_t)ev.keyboard_button_data.button] = true;
				key_held[(size_t)ev.keyboard_button_data.button] = true;
				break;

			case ksn::event_type_t::resize:
				draw_adapter.resize(ev.window_resize_data.width_new, ev.window_resize_data.height_new);
				break;
			}

		}

		cycle_fdt -= event_sw.stop().as_nsec() / 1e9f;





		//Update

		for (size_t i = 0; i < ksn::countof(key_pressed); ++i) key_held[i] |= key_pressed[i];

		leap_nanodt = sw.restart();
		float dt = leap_nanodt / 1e9f;
		cycle_fdt += dt;

		stop = !digilog_update(dt);



		//Aux
		if (((++tick_counter * fps_freq_multiplier) % window.window.get_framerate()) == 0)
		{
			char buffer[128];
			sprintf(buffer, "Digital Logic Emulator by kodirovsshik - %i FPS", int(window.window.get_framerate() / (cycle_fdt * fps_freq_multiplier)));
			window.window.set_title(buffer);

			cycle_fdt = 0;
		}
	}



	return 0;
}