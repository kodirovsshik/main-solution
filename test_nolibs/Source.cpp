
#include <Windows.h>
#include <gl/GL.h>
#include <stdint.h>

__forceinline int ___constructor()
{
	WNDCLASSA wc{};
	wc.lpfnWndProc = DefWindowProcA;
	wc.hCursor = LoadCursorA(nullptr, (LPCSTR)IDC_ARROW);
	wc.lpszClassName = "_MYCLASSNAME1NOLIB";
	ATOM x = RegisterClassA(&wc);
	return 0;
}

struct wnd_t
{
	HWND window;
	HGLRC context;
	HDC hdc;
};

__forceinline wnd_t open_window(uint16_t width, uint16_t height, const char* class_name = nullptr, const char* title = "", bool resizable = true, bool minmax = true)
{
	wnd_t result;
	DWORD style = WS_BORDER;
	if (minmax) style |= WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	if (resizable) style |= WS_THICKFRAME;

	RECT rc;
	rc.left = rc.top = 0;
	rc.right = width;
	rc.bottom = height;
	AdjustWindowRectEx(&rc, style, false, 0);

	if (class_name == nullptr) class_name = "_MYCLASSNAME1NOLIB";
	result.window = CreateWindowA(class_name, title, style, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, NULL, NULL);


	result.hdc = GetDC(result.window);
	PIXELFORMATDESCRIPTOR pfd{};
	//ppmemset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;
	SetPixelFormat(result.hdc, ChoosePixelFormat(result.hdc, &pfd), &pfd);

	result.context = wglCreateContext(result.hdc);

	if (result.window == nullptr || result.hdc == nullptr || result.context == nullptr)
	{
		wglDeleteContext(result.context);
		ReleaseDC(result.window, result.hdc);
		DestroyWindow(result.window);

		result.window = nullptr;
		result.hdc = nullptr;
		result.context = nullptr;
	}
	else
	{
		ShowWindow(result.window, SW_SHOW);
	}
	return result;
}

int mainCRTStartup()
{
#if 1 && defined _DEBUG
	__debugbreak();
#endif

	___constructor();

	HANDLE stdin = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE stdout = GetStdHandle(STD_OUTPUT_HANDLE);

	wnd_t win = open_window(800, 600);

	{
		int _;
		WriteFile(stdout, "\n", 1, nullptr, nullptr);
		(void)ReadFile(stdin, &_, 1, nullptr, nullptr);
	}
	return 0;
}
