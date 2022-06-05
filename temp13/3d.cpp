
#include <iostream>

#include <ksn/window_gl.hpp>
#include <ksn/math_vec.hpp>
#include <ksn/math_matrix.hpp>
#include <ksn/time.hpp>

#include <GL/glew.h>

#include <numeric>

#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_window_gl.lib")
#pragma comment(lib, "libksn_time.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")

struct drawee_geometry_t
{
	std::vector<ksn::vec3f> vertices;
	std::vector<ksn::vec3i> surfaces;
};

struct camera_t
{
	ksn::vec3f pos;
	ksn::matrix<3, 3> orientation = ksn::matrix<3, 3>::identity();
	float half_fov_x = NAN;
	float half_fov_y = NAN;
	float angle_ox = 0;
	float angle_oy = 0;
};

static drawee_geometry_t s_unit_cube =
{
	//vertices
	{
		{-1, -1, -1},
		{1, -1, -1},
		{1, 1, -1},
		{-1, 1, -1},
		{-1, -1, 1},
		{1, -1, 1},
		{1, 1, 1},
		{-1, 1, 1},
	},
	//surfaces
	{
		{0, 3, 2},
		{0, 2, 1},
		{0, 5, 4},
		{0, 1, 5},
		{4, 6, 7},
		{4, 5, 6},
		{3, 6, 2},
		{3, 7, 6},
		{0, 4, 7},
		{0, 7, 3},
		{1, 6, 5},
		{1, 2, 6},
	}
};

struct drawee_t
{
	drawee_geometry_t data;
	ksn::matrix<3, 3> orientation = ksn::matrix<3, 3>::identity();
	ksn::vec3f origin;
	ksn::vec3f pos;
};

void draw(const drawee_t& drawee, const camera_t& cam)
{
	auto project_vertex = [&]
	(ksn::vec3f v) -> ksn::vec2f
	{
		return { v[0] / v[1] * cam.half_fov_x, v[2] / v[1] * cam.half_fov_y};
	};

	auto preprocess_vertex = [&]
	(ksn::vec3f vertex) -> ksn::vec3f
	{
		//object geometry coordinates-> object coordinates
		vertex -= drawee.origin;
		vertex = drawee.orientation * vertex;

		//object coordinates -> world coordinates
		vertex += drawee.pos;
		//v *= drawee.rotation_around_world_origin

		//world coordinates -> camera coordinates
		vertex -= cam.pos;
		vertex = cam.orientation * vertex;

		return vertex;
	};

	static std::vector<ksn::vec3f> transformed;
	transformed.clear();
	transformed.resize(drawee.data.vertices.size());

	std::transform(drawee.data.vertices.begin(), drawee.data.vertices.end(), transformed.begin(), preprocess_vertex);

	static std::vector<ksn::vec2f> projected;
	projected.clear();
	projected.resize(transformed.size(), { NAN, NAN });

	
	glBegin(GL_LINES);

	size_t i = 0;
	for (const auto& surface : drawee.data.surfaces)
	{
		const auto [ax, ay, az] = (transformed[surface[1]] - transformed[surface[0]]).data;
		const auto [bx, by, bz] = (transformed[surface[2]] - transformed[surface[0]]).data;
		const ksn::vec3f n{ ay*bz - by*az, bx*az - ax*bz, ax*by - bx*ay };

		ksn::vec3f camera_ray = transformed[surface[0]];
		if (camera_ray[1] <= 0)
			camera_ray *= 0;

		if (camera_ray * n >= 0)
			continue;
		
		for (auto idx : surface.data)
		{
			if (isnan(projected[idx][0]))
				projected[idx] = project_vertex(transformed[idx]);

			//glVertex3f(transformed[idx][0], transformed[idx][2], -transformed[idx][1]);
		}

		glVertex2f(projected[surface[0]][0], projected[surface[0]][1]);
		glVertex2f(projected[surface[1]][0], projected[surface[1]][1]);

		glVertex2f(projected[surface[1]][0], projected[surface[1]][1]);
		glVertex2f(projected[surface[2]][0], projected[surface[2]][1]);

		glVertex2f(projected[surface[2]][0], projected[surface[2]][1]);
		glVertex2f(projected[surface[0]][0], projected[surface[0]][1]);
	}

	glEnd();
}

int main()
{
	static constexpr size_t W = 800;
	static constexpr size_t H = 600;
	//static constexpr size_t max_dim = W > H ? W : H;
	static constexpr float pi = 3.14159265f;

	ksn::window_gl_t win;

	float fov = 120;
	camera_t cam{};
	cam.pos = { 0.f, -2.5f, 0.f };
	cam.half_fov_x = 1 / tanf(fov / 2 * pi / 180);
	cam.half_fov_y = 1 / tanf(fov / 2 * pi / 180);
	

	ksn::window_style_t style{};
	style |= ksn::window_style::caption;
	style |= ksn::window_style::close_button;
	style |= ksn::window_style::hidden;

	if (win.open(W, H, "", {}, style) != ksn::window_open_result::ok)
		return 1;
	win.context_make_current();

	std::cout << glGetString(GL_VENDOR) << std::endl;
	std::cout << glGetString(GL_RENDERER) << std::endl;
	std::cout << glGetString(GL_VERSION) << std::endl;

	const uint32_t fps = 60;
	win.set_framerate(fps);



	glViewport(0, 0, W, H);

	glMatrixMode(GL_PROJECTION_MATRIX);
	glLoadIdentity();
	glOrtho(-1.f * W / H, 1.f * W / H, -1, 1, 0, 1);

	glClearColor(0, 0, 0, 0);
	glColor3ub(255, 255, 255);



	drawee_t cube;
	cube.data = s_unit_cube;



	win.show();

	ksn::stopwatch sw;
	sw.start();

	std::vector<bool> key_pressed, key_down;
	key_pressed.resize((int)ksn::keyboard_button_t::buttons_count, false);
	key_down.resize((int)ksn::keyboard_button_t::buttons_count, false);

	ksn::event_t ev;
	float dt = 1.f / fps;
	while (true)
	{
		//Poll
		for (auto&& x : key_pressed)
			x = false;

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
				case ksn::keyboard_button_t::escape:
					win.close();
					break;
				}
				key_pressed[(int)ev.keyboard_button_data.button] = true;
				key_down[(int)ev.keyboard_button_data.button] = true;
				break;

			case ksn::event_type_t::keyboard_release:
				key_down[(int)ev.keyboard_button_data.button] = false;
				break;
			}
		}

		if (!win.is_open())
			break;



		//Update
		{
			const float omega = 90 * pi / 180; //angular velocity of rotation
			float d_ox = 0;
			float d_oy = 0;

			if (key_down[(int)ksn::keyboard_button_t::arrow_left])
				d_ox -= omega;
			if (key_down[(int)ksn::keyboard_button_t::arrow_right])
				d_ox += omega;
			if (key_down[(int)ksn::keyboard_button_t::arrow_up])
				d_oy -= omega;
			if (key_down[(int)ksn::keyboard_button_t::arrow_down])
				d_oy += omega;
			cam.angle_ox += d_ox * dt;
			cam.angle_oy += d_oy * dt;

			const float cos_a = cosf(cam.angle_ox);
			const float sin_a = sinf(cam.angle_ox);
			const float cos_b = cosf(cam.angle_oy);
			const float sin_b = sinf(cam.angle_oy);
			
			cam.orientation = ksn::matrix<3, 3>{{
				{1, 0, 0},
				{0, cos_b, -sin_b},
				{0, sin_b, cos_b},
			}};
			cam.orientation *= ksn::matrix<3, 3>{{
				{ cos_a, -sin_a, 0 },
				{ sin_a, cos_a, 0 },
				{ 0, 0, 1 },
			}};



			ksn::vec3f dpos;
			if (key_down[(int)ksn::keyboard_button_t::w])
				dpos += ksn::vec3f{ 0, 1, 0};
			if (key_down[(int)ksn::keyboard_button_t::a])
				dpos += ksn::vec3f{ -1, 0, 0 };
			if (key_down[(int)ksn::keyboard_button_t::s])
				dpos += ksn::vec3f{ 0, -1, 0 };
			if (key_down[(int)ksn::keyboard_button_t::d])
				dpos += ksn::vec3f{ 1, 0, 0 };
			
			dpos = cam.orientation.inverse() * dpos;

			if (key_down[(int)ksn::keyboard_button_t::space])
				dpos += ksn::vec3f{ 0, 0, 1 };
			if (key_down[(int)ksn::keyboard_button_t::shift_left] ||
				key_down[(int)ksn::keyboard_button_t::shift_right])
				dpos += ksn::vec3f{ 0, 0, -1 };
			
			dpos.normalize();
			cam.pos += dpos * dt;
		}



		//Draw
		glClear(GL_COLOR_BUFFER_BIT);

		draw(cube, cam);



		//Display
		win.swap_buffers();
		
		

		//Sync
		win.tick();
		dt = sw.restart().as_float_sec();
	}
}
