#include<Windows.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	WNDCLASSEXA wc;
	memset(&wc, NULL, sizeof(WNDCLASSEX));
	HWND hWND;

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)WHITE_BRUSH;
	wc.hIcon = LoadIcon(NULL, IDC_ARROW);
	wc.hIconSm = LoadIcon(NULL, IDC_ARROW);
	wc.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "Window";
	wc.lpszMenuName = "Menu Name";

	if (!RegisterClassExA(&wc))
	{
		MessageBox(NULL, L"Not create Class ex", L"error", MB_OK);

	}
	hWND = CreateWindowEx(NULL, L"Hello World!", L"Name Window", WS_OVERLAPPEDWINDOW, 100, 100, 640, 480, NULL, NULL, hInstance, NULL);
	ShowWindow(hWND, nShowCmd);
	UpdateWindow(hWND);
	MSG Msg;

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWND, UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
	default:
		return DefWindowProc(hWND, uMsg, wp, lp);
		break;
	}
	return 0;
}