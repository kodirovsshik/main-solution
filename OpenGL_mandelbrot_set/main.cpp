
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#include <Windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <CL/opencl.hpp>

#include <ksn/stuff.hpp>
#include <ksn/opencl_stuff.hpp>

#include <stdlib.h>

//I'll need a bunch of those
//#pragma comment(lib, "")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")

#pragma comment(lib, "OpenCL.lib")
#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "runtimeobject.lib")

#pragma comment(lib, "libksn_stuff.lib")
#pragma comment(lib, "libksn_opencl_stuff.lib")
#pragma comment(lib, "libksn_x86_instruction_set.lib")



#define OGL_MAJOR 4
#define OGL_MINOR 6

#define str(x) #x

#define GETWNDNAME(mj, mn) "OpenGL " str(mj) "." str(mn) " Core Profile"
#define WNDNAME GETWNDNAME(OGL_MAJOR, OGL_MINOR) 



template<typename... params_t>
void nop(params_t&& ...params) {}



int main()
{
	constexpr static size_t width = 800;
	constexpr static size_t height = 600;
	

	int _glfw_init_result = glfwInit();
	ksn_dynamic_assert(_glfw_init_result == GLFW_TRUE, "Failed to initialize GLFW");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OGL_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OGL_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, WNDNAME, nullptr, nullptr);
	glfwMakeContextCurrent(window);


	glewExperimental = true;
	int _glew_init_result = glewInit();

	ksn_dynamic_assert(_glew_init_result == GLEW_OK, (const char*)glewGetErrorString(_glew_init_result));

	
	


	return 0;
}
