#include <Windows.h>
//#include <Engine.h>
#include <string>
#include <gl/GL.h>

#pragma comment(lib, "opengl32.lib")



LRESULT wEventsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 1;
}

int main()
{

	HWND handle;
	HGLRC OGLcontext;
	HDC hdc;

	//Windowclass, defines template for windows

	void* instance = nullptr;

	WNDCLASSEX wndClass;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = wEventsProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = (HINSTANCE)instance;
	wndClass.hIcon = NULL;
	wndClass.hCursor = NULL;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = L"windowClass";
	wndClass.hIconSm = NULL;

	//register window class
	if (!RegisterClassEx(&wndClass))
	{
		int _ = 0;
		_ = _;
		//FAIL
	}
	
	int width = 800;
	int height = 600;
	//Creates window and return handle to it (a way to access the windows attributes)
	handle = CreateWindowA("windowClass", "a", WS_OVERLAPPED, 0, 0, width, height, NULL, NULL, 0, NULL);

	if (handle == NULL)
	{
		return -1;
	}

	ShowWindow(handle, SW_SHOWNORMAL);
	UpdateWindow(handle);

	//pixel format description, only point worth of notice is if its 32 or 24 bit (alpha or no alpha)
	PIXELFORMATDESCRIPTOR pixelFormatDesc = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		24, 0, 0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	//Device Contex handle
	hdc = GetDC(handle); // Gets the display context

	if (hdc == NULL) {

		return -1;
	}

	int pixelFormat = ChoosePixelFormat(hdc, &pixelFormatDesc); // Chooses the pixel format

	if (pixelFormat == 0) {

		//0.o
	}

	// Sets the pixel format
	if (SetPixelFormat(hdc, pixelFormat, &pixelFormatDesc) == 0) {

		//return 0;
	}

	HGLRC hglrc = wglCreateContext(hdc); // Creates the rendering context

	if (hglrc == NULL) {

		//0.o
	}

	// Attaches the rendering context
	if (wglMakeCurrent(hdc, hglrc) == 0) {

		//return 0;
	}
}
