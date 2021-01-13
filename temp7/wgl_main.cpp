#include <Windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <ksn/ksn.hpp>
#include <ksn/stuff.hpp>

#include <conio.h>




#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")

#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_x86_instruction_set.lib")



int main()
{
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
//
//#ifdef _KSN_COMPILER_MSVC
//	wglMakeCurrent(0, 0);
//	wglDeleteContext(_wgl_context);
//	ReleaseDC(GetConsoleWindow(), _wgl_device_context);
//#endif

	const char* glfw_error_description;
	auto glfw_error = glfwGetError(&glfw_error_description);

	ksn_dynamic_assert(glew_init_result == GLEW_OK, "GLEW Initialization error %i: %s", glew_init_result, glewGetErrorString(glew_init_result));
	ksn_dynamic_assert(glfw_init_result == GLFW_TRUE, "GLFW Initialization error %i: %s", glfw_error, glfw_error_description);

	printf("Successfully created OpenGL context for console window");



	glClearColor(0, 0.1f, 0, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluOrtho2D(0.0, 500.0, 500.0, 0.0);

	glBegin(GL_POINTS);
	glEnd();

	glFlush();

	while (1)
	{
		(void)_getch();
	}

	return 0;
}
