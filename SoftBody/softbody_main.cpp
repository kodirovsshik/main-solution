
#include <ksn/window.hpp>
#include <ksn/math_vec.hpp>
#include <ksn/color.hpp>
#include <ksn/math_constants.hpp>
#include <ksn/math_complex.hpp>
#include <ksn/math_matrix.hpp>

#include <xmmintrin.h>


#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_time")


class soft_body_t
{
public:
	struct node
	{
		struct spring
		{
			float spring_coefficient;
			float stable_length;
			node* other_node;
		};

		float mass;
		ksn::vec2f position, velocity;
		std::vector<spring> springs;
	};

	std::vector<node> m_nodes;
	ksn::vec2f m_pending_force;
	float mass;
	

public:
	void force(ksn::vec2f force)
	{
		this->m_pending_force += force / this->m_nodes.size();
	}


private:
	ksn::vec2f node_force(soft_body_t::node& node)
	{
		ksn::vec2f current_force;
		for (const auto& spring : node.springs)
		{
			auto dpos = spring.other_node->position - node.position;
			current_force += spring.spring_coefficient * (dpos - spring.stable_length * dpos.normalized());
		}
		return current_force + this->m_pending_force;
	};
	ksn::vec2f node_acceleration(soft_body_t::node& node)
	{
		return node_force(node) / node.mass;
	}

	auto node_update_euler(soft_body_t::node& node, float dt, ksn::vec2f force)
	{
		node.velocity += force / node.mass * dt;
		node.position += node.velocity * dt;

		const float floor_level = 0;
		const float restitution = 0.5;
		if (node.position[1] < floor_level)
		{
			node.velocity[1] *= -restitution;
			node.position[1] *= -restitution;
		}
	};

	void _update_euler(float dt)
	{
		for (auto& node : this->m_nodes)
		{
			node_update_euler(node, dt, node_force(node));
		}
	}
	void _update_midpoint(float dt)
	{
		for (auto& node : this->m_nodes)
		{
			soft_body_t::node local_node = node;

			node_update_euler(local_node, dt / 2, node_force(local_node));
			node_update_euler(node, dt, node_force(local_node));
		}

		this->m_pending_force = {};
	}
	void _update_rk4(float dt)
	{
		size_t node_index = 0;
		for (auto& node : this->m_nodes)
		{
			soft_body_t::node local_node;

			auto k1 = node_acceleration(node);

			local_node = node;
			node_update_euler(local_node, dt / 2, k1);
			auto k2 = node_acceleration(local_node);

			local_node = node;
			node_update_euler(local_node, dt / 2, k2);
			auto k3 = node_acceleration(local_node);

			local_node = node;
			node_update_euler(local_node, dt, k3);
			auto k4 = node_acceleration(local_node);

			node_update_euler(node, dt, (k1 + 2 * k2 + 2 * k3 + k4) / 6);
			++node_index;
		}

		this->m_pending_force = {};
	}


public:
	void update(float dt)
	{
		this->_update_rk4(dt);
	}
};

int main()
{
	static constexpr size_t width = 800;
	static constexpr size_t height = 600;
	static constexpr size_t framerate_limit = 60;

	static constexpr float aspect_ratio = float(width) / height;

	ksn::window_t window;
	window.open(width, height);

	window.set_framerate(framerate_limit);
	

	auto& screen = *(ksn::color_bgr_t(*)[height][width])new ksn::color_bgr_t[height * width];



	std::vector<soft_body_t> bodies;

	if constexpr (true)
	{
		const float w = 100, h = 100;
		const float mass = 10;
		const float spread_rate = 20;
		const int blocks_w = int(w / spread_rate) + 1;
		const int blocks_h = int(h / spread_rate) + 1;
		const int blocks = blocks_w * blocks_h;
		const float mass_per_block = mass / blocks;
		const float spread_rate_w = w / blocks_w;
		const float spread_rate_h = h / blocks_h;
		const float spread_rate_diag = hypotf(spread_rate_w, spread_rate_h);
		const float x_start = width / 2 - w / 2;
		//const float y_start = height / 2 - h / 2;
		const float y_start = 500;
		const float K = 70; //sretch resistance coefficient

		soft_body_t body;
		body.m_nodes.resize(blocks);

		soft_body_t::node::spring current_spring;
		current_spring.spring_coefficient = K;

		body.mass = mass;

		for (int i = 0; i < blocks_h; ++i)
		{
			for (int j = 0; j < blocks_w; ++j)
			{
				auto& node = body.m_nodes[i * blocks_w + j];
				node.mass = mass_per_block;
				node.position = { x_start + j * spread_rate_w, y_start + i * spread_rate_h };
				node.springs.reserve(4);

				if (i > 0)
				{
					current_spring.other_node = &body.m_nodes[(i - 1) * blocks_w + j];
					current_spring.stable_length = spread_rate_h;
					node.springs.push_back(current_spring);
				}
				if (j > 0)
				{
					current_spring.other_node = &body.m_nodes[i * blocks_w + j - 1];
					current_spring.stable_length = spread_rate_w;
					node.springs.push_back(current_spring);
				}
				if (i < blocks_h - 1)
				{
					current_spring.other_node = &body.m_nodes[(i + 1) * blocks_w + j];
					current_spring.stable_length = spread_rate_h;
					node.springs.push_back(current_spring);
				}
				if (j < blocks_w - 1)
				{
					current_spring.other_node = &body.m_nodes[i * blocks_w + j + 1];
					current_spring.stable_length = spread_rate_w;
					node.springs.push_back(current_spring);
				}
				if (i > 0 && j > 0)
				{
					current_spring.other_node = &body.m_nodes[(i - 1) * blocks_w + j - 1];
					current_spring.stable_length = spread_rate_diag;
					node.springs.push_back(current_spring);
				}
				if (i < blocks_h - 1 && j > 0)
				{
					current_spring.other_node = &body.m_nodes[(i + 1) * blocks_w + j - 1];
					current_spring.stable_length = spread_rate_diag;
					node.springs.push_back(current_spring);
				}
				if (i < blocks_h - 1 && j < blocks_w - 1)
				{
					current_spring.other_node = &body.m_nodes[(i + 1) * blocks_w + j + 1];
					current_spring.stable_length = spread_rate_diag;
					node.springs.push_back(current_spring);
				}
				if (i > 0 && j < blocks_w - 1)
				{
					current_spring.other_node = &body.m_nodes[(i - 1) * blocks_w + j + 1];
					current_spring.stable_length = spread_rate_diag;
					node.springs.push_back(current_spring);
				}
			}
		}

		bodies.push_back(std::move(body));
	}



	bool pause = true;
	bool pause_skip = false;

	while (true)
	{
		memset(screen, 0, sizeof(screen));

		for (const auto& body : bodies)
		{
			for (const auto& node : body.m_nodes)
			{
				float radius = 1.5;

				float node_width = 2 * radius, node_height = node_width;

				int x_start = int(node.position[0] - radius) + 1;
				int y_start = int(node.position[1] - radius) + 1;

				int x_end = x_start + (int)node_width;
				int y_end = y_start + (int)node_height;

				if (x_start < 0) x_start = 0;
				if (y_start < 0) y_start = 0;

				if (x_end > (float)width) x_end = width;
				if (y_end > (float)height) y_end = height;

				ksn::color_bgr_t node_filler(0x00FF00);
				ksn::color_bgr_t spring_filler(0x777777);

				for (int y = y_start; y < y_end; ++y)
				{
					for (int x = x_start; x < x_end; ++x)
					{
						screen[height - 1 - y][x] = node_filler;
					}
				}

				for (const auto& spring : node.springs)
				{
					if (&node < spring.other_node)
					{
						auto dpos = spring.other_node->position - node.position;

						float flen = dpos.abs();
						int length = int(flen + 0.5f);
						
						dpos.normalize();
						auto pos = node.position;
						for (int i = 0; i < length; ++i)
						{
							int x = int(pos[0] + 0.5f);
							int y = int(height - 0.5f - pos[1]);
							if (x >= 0 && x < width && y >= 0 && y < height)
								screen[y][x] = spring_filler;
							pos += dpos;
						}
					}
				}
			}
		}

		window.draw_pixels_bgr_front(screen);
		window.tick();


		ksn::event_t ev;
		while (window.poll_event(ev))
		{
			switch (ev.type)
			{
			case ksn::event_type_t::close:
				window.close();
				break;

			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::esc:
					window.close();
					break;

				case ksn::keyboard_button_t::space:
					pause = !pause;
					break;

				case ksn::keyboard_button_t::arrow_up:
					if (pause)
						pause_skip = true;
					break;
				}
				break;
			}
		}

		if (!window.is_open())
			break;



		if (!pause || pause_skip)
		{
			const float g = 9.8f * 10;

			for (auto& body : bodies)
			{
				const float update_time = 1.f / framerate_limit;
				float time_step = 1;

				int time_steps_count = int(update_time / time_step + 0.5f);
				if (time_steps_count == 0) time_steps_count = 1;
				time_step = update_time / time_steps_count;

				while (time_steps_count --> 0)
				{
					body.force({ 0.f, -g * body.mass });
					body.update(time_step);
				}
			}

			if (pause)
				pause_skip = false;
		}
	}
}
