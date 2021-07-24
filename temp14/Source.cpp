
#include <GL/glew.h>

#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_TARGET_OPENCL_VERSION 120
#define CL_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_EXCEPTIONS

#include <ksn/window_gl.hpp>
#include <ksn/opencl_selector.hpp>
#include <ksn/stuff.hpp>

#include <CL/opencl.hpp>

#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_window_gl")
#pragma comment(lib, "libksn_opencl_selector")
#pragma comment(lib, "libksn_time")
#pragma comment(lib, "libksn_stuff")
#pragma comment(lib, "libksn_image")
#pragma comment(lib, "libksn_crc")

#pragma comment(lib, "zlibstatic")

#pragma comment(lib, "OpenCL")
#pragma comment(lib, "runtimeobject")
#pragma comment(lib, "Cfgmgr32")

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glew32s")









const std::string cl_src = R"(

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST;

__kernel void kernel_img_read(__read_only image2d_t in)
{
	size_t id = get_global_id(0);
	int width = get_image_width(in);
	int2 pos = (int2)(id % width, id / width);

	float4 data1 = read_imagef(in, sampler, pos);
	if (id == 0)
		printf("%g %g %g %g\n", data1.x, data1.y, data1.z, data1.w);
}

__kernel void kernel_fill_blue(__write_only image2d_t in)
{
	size_t id = get_global_id(0);
	int width = get_image_width(in);
	int2 pos = (int2)(id % width, id / width);

	write_imagef(in, pos, (float4)(0.f, 0.f, 0.97f, 0.f));
}

)";


void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
	static_assert(std::is_same_v<GLchar, char>);

	fprintf(stderr, "\ngl_error_callback() was called:\n%.*s\n", (int)length, (const char*)message);
	__debugbreak();
}


#define glCheck() if constexpr (true) { int err = glGetError(); if (err) return err; } else ksn::nop()

int main()
{
	constexpr uint16_t width = 200;
	constexpr uint16_t height = 200;

	cl_int clerr;

	ksn::window_gl_t win;

	//Window & OpenGL context initialization
	if constexpr (true)
	{
		ksn::window_gl_t::context_settings context_settings{};
		context_settings.bits_per_color = 24;
		context_settings.ogl_compatibility_profile = true;
		context_settings.ogl_debug = true;
		context_settings.ogl_version_major = 3;
		context_settings.ogl_version_minor = 0;

		ksn::window_style_t window_style = 0;
		window_style |= ksn::window_style::border;
		window_style |= ksn::window_style::caption;
		window_style |= ksn::window_style::close_button;
		window_style |= ksn::window_style::hidden;

		if (win.open(width, height, L"", context_settings, window_style) != ksn::window_open_result::ok)
			return 1;

		win.context_make_current();

		glDebugMessageCallback(gl_error_callback, 0);
		glCheck();
	}

	
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	cl::Platform platform;
	cl::Context context;
	cl::Device device;
	cl::Program program;
	cl::CommandQueue q;
	

	//OpenCL initialization
	if constexpr (true)
	{
		std::string ext_string;

		bool ok_context = false;

		for (auto& current_platform : platforms)
		{
			current_platform.getInfo(CL_PLATFORM_EXTENSIONS, &ext_string);
			if (ext_string.find("cl_khr_gl_sharing") == std::string::npos)
				continue;

			cl_context_properties properties[] =
			{
				CL_GL_CONTEXT_KHR, (cl_context_properties)win.context_native_handle(),
				CL_WGL_HDC_KHR, (cl_context_properties)win.winapi_get_hdc(),
				CL_CONTEXT_PLATFORM, (cl_context_properties)current_platform(),
				0
			};

			std::vector<cl::Device> devices;
			current_platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

			for (auto& current_device : devices)
			{
				current_device.getInfo(CL_DEVICE_EXTENSIONS, &ext_string);
				if (ext_string.find("cl_khr_gl_sharing") == std::string::npos)
					continue;

				try
				{
					context = cl::Context(current_device, properties);
				}
				catch (...)
				{
					continue;
				}

				ok_context = true;
				device = std::move(current_device);
				break;
			}

			if (!ok_context) continue;

			platform = std::move(current_platform);
			break;
		}

		if (!ok_context)
			return 2;
	}

	try
	{
		program = cl::Program(context, cl_src, true);
	}
	catch (const cl::BuildError& berr)
	{
		fprintf(stderr, "Build status: %i\n", (int)berr.err());

		const auto& log = berr.getBuildLog();
		for (const auto& entry : log)
		{
			fprintf(stderr, "\n%s\n", entry.second.c_str());
		}
		return 1;
	}

	q = cl::CommandQueue(context, device);

	clerr = device.getInfo<CL_DEVICE_IMAGE_SUPPORT>();

	


	GLuint fbo = -1, renderbuffer = -1;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	glCheck();

	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glCheck();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glCheck();




	cl_mem cl_gl_buff = clCreateFromGLRenderbuffer(context(), CL_MEM_READ_WRITE, renderbuffer, &clerr);
	cl::Image2D rbuff(cl_gl_buff, true);

	cl::Kernel kernel_fill_blue(program, "kernel_fill_blue");
	cl::Kernel kernel_img_read(program, "kernel_img_read");

	kernel_fill_blue.setArg(0, rbuff);
	kernel_img_read.setArg(0, rbuff);

	std::vector<cl_mem> cl_gl_mem_handlers = { rbuff() };




	win.set_framerate(60);
	win.show();

	ksn::event_t ev;
	while (true)
	{
		//Render

		//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		//glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);

		glClearColor(1, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glFinish();
		clEnqueueAcquireGLObjects(q(), (unsigned)cl_gl_mem_handlers.size(), cl_gl_mem_handlers.data(), 0, 0, 0);
	
		q.enqueueNDRangeKernel(kernel_fill_blue, 0, width * height);
		
		q.finish();
		clEnqueueReleaseGLObjects(q(), (unsigned)cl_gl_mem_handlers.size(), cl_gl_mem_handlers.data(), 0, 0, 0);

		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);

		win.swap_buffers();
		win.tick();



		//Poll events
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
				}
				break;
			}
		}

		if (!win.is_open())
			break;



		//Update

	}


	return 0;
}