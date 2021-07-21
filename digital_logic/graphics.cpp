
#include "graphics.hpp"
#include "opencl.hpp"





draw_adapter_t draw_adapter;





draw_adapter_t::draw_adapter_t() noexcept
{

}

draw_adapter_t::draw_adapter_t(draw_adapter_t&& x) noexcept
{
	std::swap(this->m_screen_data, x.m_screen_data);
}

void draw_adapter_t::resize(uint16_t x, uint16_t y)
{
	this->m_screen_data.resize((size_t)x * y);
	this->m_width = x;
	this->m_height = y;
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

void draw_adapter_t::display(ksn::window_t& win)
{
	const cl::Buffer* p_buffer = &this->m_screen_videodata;
	if (this->m_scaling > 1)
	{
		cl_data.kernel_downscale.setArg(0, this->m_screen_videodata);
		cl_data.kernel_downscale.setArg(1, this->m_screen_videodata_downscaled);
		cl_data.kernel_downscale.setArg(2, this->m_width);
		cl_data.kernel_downscale.setArg(3, this->m_height);
		cl_data.kernel_downscale.setArg(4, this->m_scaling);
		cl_data.q.enqueueNDRangeKernel(cl_data.kernel_downscale, cl::NullRange, cl::NDRange((size_t)this->m_width * this->m_height));
		p_buffer = &this->m_screen_videodata_downscaled;
	}

	cl_data.q.enqueueReadBuffer(*p_buffer, CL_TRUE, 0, (cl::size_type)this->m_width * this->m_height * sizeof(ksn::color_bgra_t), this->m_screen_data.data());
	win.draw_pixels_bgra_front(this->m_screen_data.data());
}

void draw_adapter_t::clear(ksn::color_bgra_t color)
{
	const cl::Buffer* p_buffer = &this->m_screen_videodata;
	if (this->m_scaling > 1) p_buffer = &this->m_screen_videodata_downscaled;
	cl_data.kernel_clear.setArg(0, *p_buffer);
	cl_data.kernel_clear.setArg(1, color);
	cl_data.kernel_clear.setArg(2, this->m_width);
	cl_data.kernel_clear.setArg(3, this->m_scaling);
	cl_data.q.enqueueNDRangeKernel(cl_data.kernel_clear, cl::NullRange, cl::NDRange((size_t)this->m_width * this->m_height));
}

void draw_adapter_t::update_video_buffers()
{
	cl::size_type capacity = (cl::size_type)sizeof(ksn::color_bgra_t) * this->m_width * this->m_height;

	this->m_screen_videodata_downscaled= this->m_scaling > 1 ?
		cl::Buffer(cl_data.context, CL_MEM_READ_WRITE, capacity * this->m_scaling, nullptr) :
		cl::Buffer();

	this->m_screen_videodata =
		cl::Buffer(cl_data.context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, capacity, this->m_screen_data.data());
}

