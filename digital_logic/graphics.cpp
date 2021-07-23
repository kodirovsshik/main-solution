
#include "graphics.hpp"
#include "opencl.hpp"
#include "window.hpp"
#include "err_handling.hpp"





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
	this->m_size[0] = x;
	this->m_size[1] = y;
	this->update_video_buffers();
}

void draw_adapter_t::set_image_scaling(uint8_t n)
{
	static constexpr int scaling_max = 255;
	if (n > scaling_max)
	{
		fprintf(stderr, "Invalid scaling coefficient %i, maximum of %i is allowed\n", (int)n, scaling_max);
		return;
	}

	this->m_scaling = n;
	this->update_video_buffers();
}

void draw_adapter_t::display(ksn::window_t& win)
{
	const cl::Buffer* p_buffer = &this->m_screen_videodata;
	if (this->m_scaling > 1)
	{
		cl_data.kernel_downscale.setArg(0, this->m_screen_videodata);
		cl_data.kernel_downscale.setArg(1, this->m_screen_videodata_downscaled);
		cl_data.kernel_downscale.setArg(2, this->m_size[0]);
		cl_data.kernel_downscale.setArg(3, this->m_size[1]);
		cl_data.kernel_downscale.setArg(4, this->m_scaling);
		cl_data.q.enqueueNDRangeKernel(cl_data.kernel_downscale, cl::NullRange, cl::NDRange((size_t)this->m_size[0] * this->m_size[1]));
		p_buffer = &this->m_screen_videodata_downscaled;
	}

	cl_data.q.enqueueReadBuffer(*p_buffer, CL_TRUE, 0, (cl::size_type)this->m_size[0] * this->m_size[1] * sizeof(ksn::color_bgra_t), this->m_screen_data.data());
	win.draw_pixels_bgra_front(this->m_screen_data.data());
}

void draw_adapter_t::draw(const object_t& obj) const
{
	cl_data.kernel_draw_sprite_default.setArg(0, obj.m_texture->m_videodata);
	cl_data.kernel_draw_sprite_default.setArg(1, this->m_screen_videodata);
	cl_data.kernel_draw_sprite_default.setArg(2, obj.m_transform_data);
	cl_data.kernel_draw_sprite_default.setArg(3, obj.m_sprite_space_data);
	cl_data.kernel_draw_sprite_default.setArg(4, this->m_size);
	cl_data.kernel_draw_sprite_default.setArg(5, this->m_scaling);
	
	auto& spsize = obj.m_sprite_space_data.m_sprite_size;
	
	size_t sprite_size = (size_t)spsize[0] * spsize[1];
	cl_data.q.enqueueNDRangeKernel(cl_data.kernel_draw_sprite_default, cl::NullRange, cl::NDRange(ksn::align_up(sprite_size, cl_data.max_work_group_size)), cl::NDRange(cl_data.max_work_group_size));
}

void draw_adapter_t::clear(ksn::color_bgra_t color)
{
	const cl::Buffer* p_buffer = &this->m_screen_videodata;
	cl_data.kernel_clear.setArg(0, *p_buffer);
	cl_data.kernel_clear.setArg(1, color);
	cl_data.kernel_clear.setArg(2, this->m_size[0]);
	cl_data.kernel_clear.setArg(3, this->m_scaling);
	cl_data.q.enqueueNDRangeKernel(cl_data.kernel_clear, cl::NullRange, cl::NDRange((size_t)this->m_size[0] * this->m_size[1]));
}

void draw_adapter_t::update_video_buffers()
{
	cl::size_type capacity = (cl::size_type)sizeof(ksn::color_bgra_t) * this->m_size[0] * this->m_size[1];

	this->m_screen_videodata_downscaled = this->m_scaling > 1 ?
		cl::Buffer(cl_data.context, CL_MEM_READ_WRITE, capacity) :
		cl::Buffer();

	this->m_screen_videodata =
		cl::Buffer(cl_data.context, CL_MEM_READ_WRITE, capacity * this->m_scaling * this->m_scaling);
}



void object_t::set_sprite(const texture_t* t, ksn::vec<2, uint16_t> position, ksn::vec<2, uint16_t> size)
{
	this->m_texture = t;
	this->m_sprite_space_data.m_sprite_size = size;
	this->m_sprite_space_data.m_sprite_texture_offset = position;
	this->m_sprite_space_data.m_texture_size = t ? ksn::vec<2, uint16_t>{t->m_width, t->m_height} : ksn::vec<2, uint16_t>{0, 0};
}



ksn::image_bgra_t::load_result_t texture_t::load(const char* path)
{
	ksn::image_bgra_t temp_image;
	auto result = temp_image.load_from_file(path);
	critical_assert1(result == ksn::image_bgra_t::load_result::ok, (int)result, "Texture load failure", "Failed to load texture \"%s\", ksn::image_load_result_t = %i", path, (int)result);

	size_t buff_size = sizeof(ksn::color_bgra_t) * temp_image.width * temp_image.height;

	this->m_videodata = cl::Buffer(); //free the previous buffer first
	this->m_videodata = cl::Buffer(cl_data.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buff_size, temp_image.m_data.data());

	this->m_width = temp_image.width;
	this->m_height = temp_image.height;

	return result;
}

//Sets the rotaion angle
void object_t::set_rotation(float angle) noexcept
{
	this->m_transform_data.m_rotation_data = { sinf(angle), cosf(angle) };
}
void object_t::set_rotation_degrees(float angle) noexcept
{
	this->set_rotation(angle / 180 * KSN_PIf);
}

//Adds the specified angle to the current one
void object_t::rotate(float delta_angle) noexcept
{
	float vsin = sinf(delta_angle);
	float vcos = cosf(delta_angle);

	float tsin = this->m_transform_data.m_rotation_data[0] * vcos + this->m_transform_data.m_rotation_data[1] * vsin;
	float tcos = this->m_transform_data.m_rotation_data[1] * vcos - this->m_transform_data.m_rotation_data[0] * vsin;

	this->m_transform_data .m_rotation_data= { tsin, tcos };
}
void object_t::rotate_degrees(float delta_angle) noexcept
{
	this->rotate(delta_angle / 180 * KSN_PIf);
}

//Returns rotation angle
float object_t::get_rotation() const noexcept
{
	return atan2f(this->m_transform_data.m_rotation_data[0], this->m_transform_data.m_rotation_data[1]);
}
float object_t::get_rotation_degrees() const noexcept
{
	return this->get_rotation() * 180 / KSN_PIf;
}
