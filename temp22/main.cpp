
#include <ksn/window.hpp>
#include <ksn/image.hpp>

#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_time")
#pragma comment(lib, "libksn_crc")
#pragma comment(lib, "libksn_stuff")
#pragma comment(lib, "zlibstatic")

#include <mdspan>

int main()
{
	ksn::window_t win;
	win.open(150, 200, "", ksn::window_style::hidden);
	win.show();

	ksn::image_t<ksn::color_bgr_t> img;
	img.load_from_file("a.png");

	const uint16_t w = img.width, h = img.height;
	win.draw_pixels_bgr_front(img.m_data.data(), 0, 0, w, h);

	std::vector<uint8_t> data(img.m_data.size());
	for (size_t i = 0; i < h; ++i)
		for (size_t j = 0; j < w; ++j)
			data[i * w + j] = img.m_data[i * w + j].b;

	auto at = [&](size_t i, size_t j) { return i * w + j; };
	auto convolve = [&](size_t i, size_t j)
	{
		int x = 0;
		x += data[at(i - 1, j - 1)];
		x += data[at(i - 1, j + 1)];
		x += data[at(i, j)];
		x += data[at(i + 1, j - 1)];
		x += data[at(i + 1, j + 1)];
		return (uint8_t)((x + 2) / 5);
	};

	for (size_t i = 1; i < h - 1; ++i)
		for (size_t j = 1; j < w - 1; ++j)
			img.m_data[i * w + j] = convolve(i, j) * 0x00010101;

	win.draw_pixels_bgr_front(img.m_data.data(), 0, 0, w, h);
}
