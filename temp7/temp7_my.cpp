#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>





#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")



#define OGL_MAJOR 3
#define OGL_MINOR 3

#define str(x) #x

#define GETWNDNAME(mj, mn) "OpenGL " str(mj) "." str(mn) " Core Profile"
#define WNDNAME GETWNDNAME(OGL_MAJOR, OGL_MINOR) 

void блять(const char* = "блять");

int main()
{
	constexpr static size_t width = 800;
	constexpr static size_t height = 600;
	constexpr static GLfloat triangle_vertex_buffer_data[] =
	{
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f,
	};

	if (!glfwInit()) блять();



	//glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OGL_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OGL_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, WNDNAME, nullptr, nullptr);
	if (window == nullptr)
	{
		const char* error;
		glfwGetError(&error);
		блять(error);
	}
	glfwMakeContextCurrent(window);



	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) блять();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	while (1)
	{
		glfwSwapBuffers(window);
		glfwPollEvents();


		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(window))
		{
			glfwDestroyWindow(window);
			window = nullptr;
		}


		if (window == nullptr) break;


		//Do work
	}


	glfwTerminate();
	return 0;
}






void блять(const char* message)
{
	MessageBoxA(GetConsoleWindow(), message, "блять", MB_ICONERROR);
	exit(-1);
}
