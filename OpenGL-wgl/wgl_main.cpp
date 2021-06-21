
#include <ksn/window.hpp>
#include <ksn/stuff.hpp>
#include <ksn/try_smol_buffer.hpp>
#include <ksn/math_constants.hpp>
#include <ksn/graphics.hpp> //for color structure(s)

#include <chrono>

#include <GL/glew.h>

#include <Windows.h>



#pragma warning (disable : 26451 4244)



//#pragma comment(lib, "libksn_window.lib")
//#pragma comment(lib, "libksn_stuff.lib")
//#pragma comment(lib, "libksn_x86_instruction_set.lib")

#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLU32.lib")



struct vertex_t
{
	float x, y;
};



unsigned int create_shader(const char* src, unsigned int type)
{
	unsigned int shader = glCreateShader(type);
	int len = (int)strlen(src);
	glShaderSource(shader, 1, &src, &len);
	glCompileShader(shader);

	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		int len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

		ksn::try_smol_buffer<char, 512> buff(len);
		glGetShaderInfoLog(shader, len + 1, &len, buff);
		
		const char* type_str = "other";
		if (type == GL_VERTEX_SHADER)
			type_str = "GL_VERTEX_SHADER";
		else if (type == GL_FRAGMENT_SHADER)
			type_str = "GL_FRAGMENT_SHADER";

		printf("Failed to compile shader of type %s:\n%s\n", type_str, buff.data());

		glDeleteShader(shader);
		shader = -1;
	}

	return shader;
}

unsigned int create_program(const char* src_vertex, const char* src_fragment)
{
	unsigned int shader_vertex = create_shader(src_vertex, GL_VERTEX_SHADER);
	if (shader_vertex == -1) return -1;
	unsigned int shader_fragment = create_shader(src_fragment, GL_FRAGMENT_SHADER);
	if (shader_fragment == -1) return -1;

	unsigned int program = glCreateProgram();

	glAttachShader(program, shader_vertex);
	glAttachShader(program, shader_fragment);

	glLinkProgram(program);

	/*int status;
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	*/
	
	return program;
}



const char source_vertex_shader[] = R"(

#version 330 core

layout(location = 0) in vec4 position;

void main()
{
	gl_Position = position;
}

)";

const char source_fragment_shader[] = R"(

#version 330 core

out vec4 color;

uniform vec4 u_color;

void main()
{
	color = u_color;
	//color = vec4(0, 1, 0, 1);
}

)";



void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei msg_len, const GLchar* msg, const void* param)
{
	printf("OpenGL error callback [%u]: %s\n", id, msg);
	int i = 0;
	i /= i;
}

#define glPollErrors() while (int err = glGetError()) { printf("OpenGL error %i\n", err); }



int main()
{
	uint16_t width = 800, height = 600;
	int temp = 0;

	ksn::window_t::context_settings ogl{ 4, 3, 24, false,
#if _KSN_IS_DEBUG_BUILD
		true
#endif
	};
	ksn::window_t win;

	if (win.open(width, height, "OpenGL 4.3 Core Profile", ogl, ksn::window_t::style::close_button) != ksn::window_t::error::ok) return 1;
	win.make_current();
	
	glewExperimental = true;
	if (glewInit() != GLEW_OK) return -1;

	printf("%s\n", glGetString(GL_VENDOR));
	printf("%s\n", glGetString(GL_RENDERER));
	printf("%s\n", glGetString(GL_VERSION));

	glDebugMessageCallback(gl_debug_callback, nullptr);



	std::vector<vertex_t> positions
	{
		{ -0.5f, -0.5f },
		{  0.5f,  -0.5f },
		{  0.0f,  0.5f },
		//{  0.5f, -0.5f },
	};
	std::vector<uint32_t> indices{ 0, 1, 2 };



	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(vertex_t), positions.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)offsetof(vertex_t, x));

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);


	unsigned int shader_program = create_program(source_vertex_shader, source_fragment_shader);
	if (shader_program == -1) return 3;
	glUseProgram(shader_program);



	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);



	std::vector<bool> key_pressed((int)ksn::keyboard_button_t::buttons_count, false);

	auto clock_f = std::chrono::high_resolution_clock::now;
	auto t = clock_f();

	float dt = 0;

	float hue = 0;
	float saturation = 1;
	float value = 1;

	float dhue = 30.f / 360;
	float dsaturation = 1;
	float dvalue = 1;

	ksn::graphics::color_rgb_t color_rgb;

	win.set_vsync_enabled(true);
	while (win.is_open())
	{
		//Draw

		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);


		glClearColor(0.1f, 0.1f, 0.1f, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		color_rgb = ksn::graphics::color_hsv_t(hue * 360, saturation * 100, value * 100);
		glUniform4f(glGetUniformLocation(shader_program, "u_color"), color_rgb.r / 255.f, color_rgb.g / 255.f, color_rgb.b / 255.f, color_rgb.a / 255.f);
		
		glDrawElements(GL_TRIANGLES, (GLsizei)indices.count(), GL_UNSIGNED_INT, nullptr);


		//Swap & sleep
		win.swap_buffers();

		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);





		auto t1 = clock_f();
		dt = (float)(t1 - t).count() / 1e9f;
		t = t1;

		//Process events

		ksn::event_t ev;
		while (win.poll_event(ev))
		{
			if (ev.type == ksn::event_type_t::close) win.close();
			else if (ev.type == ksn::event_type_t::keyboard_press)
			{
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::esc:
					win.close();
					break;
				}

				key_pressed[(int)ev.keyboard_button_data.button] = true;
			}
			else if (ev.type == ksn::event_type_t::keyboard_release)
			{
				switch (ev.keyboard_button_data.button)
				{
				default:;
				}

				key_pressed[(int)ev.keyboard_button_data.button] = false;
			}
		}

		if (key_pressed[(int)ksn::keyboard_button_t::numpad8]) hue = 0;
		if (key_pressed[(int)ksn::keyboard_button_t::numpad5]) saturation = 1;
		if (key_pressed[(int)ksn::keyboard_button_t::numpad2]) value = 1;
		if (key_pressed[(int)ksn::keyboard_button_t::numpad7])
		{
			hue -= dhue * dt;
			hue = fmodf(hue, 1);
		}
		if (key_pressed[(int)ksn::keyboard_button_t::numpad9])
		{
			hue += dhue * dt;
			hue = fmodf(hue, 1);
		}
		if (key_pressed[(int)ksn::keyboard_button_t::numpad4])
		{
			saturation -= dsaturation * dt;
			saturation = std::clamp(saturation, 0.f, 1.f);
		}
		if (key_pressed[(int)ksn::keyboard_button_t::numpad6])
		{
			saturation += dsaturation * dt;
			saturation = std::clamp(saturation, 0.f, 1.f);
		}
		if (key_pressed[(int)ksn::keyboard_button_t::numpad1])
		{
			value -= dvalue * dt;
			value = std::clamp(value, 0.f, 1.f);
		}
		if (key_pressed[(int)ksn::keyboard_button_t::numpad3])
		{
			value += dvalue * dt;
			value = std::clamp(value, 0.f, 1.f);
		}
		if (key_pressed[(int)ksn::keyboard_button_t::shift_left])
		{
			dhue = 0.5;
			dsaturation = 2;
			dvalue = 2;
		}
		else
		{
			dhue = 30.f / 360;
			dsaturation = 1;
			dvalue = 1;
		}
	}

	glDeleteProgram(shader_program);
	glDeleteBuffers(1, &vbo);

	printf("\n");

	return 0;
}
