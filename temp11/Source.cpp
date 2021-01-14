#include <Windows.h>

int main()
{
	HDC hdc = GetDC(GetConsoleWindow());

	for (size_t i = 0; i < 256; ++i)
	{
		for (size_t j = 0; j < 256; ++j)
		{
			SetPixel(hdc, i, j, i | (j << 16));
		}
	}


	return 0;
}