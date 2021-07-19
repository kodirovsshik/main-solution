
#include "graphics.hpp"
#include "opencl.hpp"



draw_adapter_t::draw_adapter_t() noexcept
{

}

draw_adapter_t::draw_adapter_t(draw_adapter_t&& x) noexcept
{
	std::swap(this->m_screen_data, x.m_screen_data);
}

void draw_adapter_t::resize(uint16_t x, uint16_t y)
{
	this->m_screen_data.resize(this->m_capacity = x * y);
	this->update_video_buffers();
}

void draw_adapter_t::set_image_scaling(uint8_t n)
{
	if (n > 4)
	{
		fprintf(stderr, "Invalid scaling power %i\n", (int)n);
		return;
	}

	this->m_scaling = n;
}

void draw_adapter_t::display(ksn::window_t& win) const noexcept
{
	if (this->m_scaling)
	{
		
	}
	else
	{

	}
	win.draw_pixels_bgra_front(this->m_screen_data.data());
}

void draw_adapter_t::update_video_buffers()
{
	this->m_screen_videodata = this->m_scaling > 1 ?
		cl::Buffer(cl_data.context, CL_MEM_READ_WRITE, this->m_capacity * sizeof(ksn::color_bgra_t) * this->m_scaling, nullptr) :
		cl::Buffer();

	this->m_screen_videodata_downscaled = 
		cl::Buffer(cl_data.context, CL_MEM_READ_WRITE, this->m_capacity * sizeof(ksn::color_bgra_t), nullptr);
}

