#include <Windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <ksn/ksn.hpp>
#include <ksn/stuff.hpp>

#include <conio.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>



#pragma warning(disable : 4996)



#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")

#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_x86_instruction_set.lib")



#define OGL_MAJOR 3
#define OGL_MINOR 3

#define str(x) #x

#define GETWNDNAME(mj, mn) "OpenGL " str(mj) "." str(mn) " Core Profile"
#define WNDNAME GETWNDNAME(OGL_MAJOR, OGL_MINOR)



int main()
{
	constexpr static int width = 800;
	constexpr static int height = 600;
	constexpr static int framerate = 60;

	FILE* f = fopen("x.txt", "w");
	setvbuf(f, nullptr, _IONBF, 0);
	//setvbuf(f, nullptr, _IOFBF, 32768);

#ifdef _KSN_COMPILER_MSVC
	HDC _wgl_device_context;
	HGLRC _wgl_context;
	{
		_wgl_device_context = GetDC(GetConsoleWindow());
		
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));

		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 24;
		pfd.iLayerType = PFD_MAIN_PLANE;
		//pfd.cStencilBits = 8;

		auto pixel_format = ChoosePixelFormat(_wgl_device_context, &pfd);
		ksn_dynamic_assert(pixel_format != 0, "ChoosePixelFormat has failed with code %i", GetLastError());
		ksn_dynamic_assert(SetPixelFormat(_wgl_device_context, pixel_format, &pfd) == TRUE, "SetPixelFormat has failed with code % i", GetLastError());

		_wgl_context = wglCreateContext(_wgl_device_context);
		
		ksn_dynamic_assert(_wgl_context != nullptr, "WGL error %i", (int)GetLastError());

		wglMakeCurrent(_wgl_device_context, _wgl_context);
	}
#endif

	glewExperimental = true;

	auto glfw_init_result = glfwInit();
	auto glew_init_result = glewInit();



#ifdef _KSN_COMPILER_MSVC
	wglMakeCurrent(0, 0);
	wglDeleteContext(_wgl_context);
	ReleaseDC(GetConsoleWindow(), _wgl_device_context);
#endif

	const char* glfw_error_description;
	auto glfw_error = glfwGetError(&glfw_error_description);

	ksn_dynamic_assert(glew_init_result == GLEW_OK, "GLEW Initialization error %i: %s", glew_init_result, glewGetErrorString(glew_init_result));
	ksn_dynamic_assert(glfw_init_result == GLFW_TRUE, "GLFW Initialization error %i: %s", glfw_error, glfw_error_description);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OGL_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OGL_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	
	GLFWwindow* window = glfwCreateWindow(800, 600, WNDNAME, 0, 0);
	
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h) 
		{
			glViewport(0, h, w, 0);
		}
	);
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int what, int mods)
		{
			if (key == GLFW_KEY_ESCAPE)
			{
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
		}
	);
	
	glfwMakeContextCurrent(window);
	glViewport(0, height, width, 0);
	glDisable(GL_DEPTH_TEST);



	uint8_t(&data)[height][width][3] = *(uint8_t(*)[height][width][3])malloc(sizeof(uint8_t[height][width][3]));
	memset(data, 0xFF, sizeof(data));

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		//Wait for previous frame to end
		[](int fps) -> void
		{
			using namespace std::chrono;
			auto clock_f = &steady_clock::now;

			static auto last_time_point = clock_f();
			
			auto now = clock_f();

			std::this_thread::sleep_for(nanoseconds(1000000000 / fps) - (now - last_time_point));
			last_time_point = now;

		}(framerate);


		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		for (size_t y = 0; y < height; ++y)
		{
			for (size_t x = 0; x < width; ++x)
			{
				data[y][x][0] = (rand() % 256) * 256 * 256 * 256;
				data[y][x][1] = (rand() % 256) * 256 * 256 * 256;
				data[y][x][2] = (rand() % 256) * 256 * 256 * 256;
			}
		}

		glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

		glfwSwapBuffers(window);
		
	}

	//glfwDestroyWindow(window);
	glfwTerminate();
	
	return 0;
}
