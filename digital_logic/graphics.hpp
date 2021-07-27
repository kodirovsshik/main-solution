
#ifndef _DIGILOG_GRAPHICS_HPP_
#define _DIGILOG_GRAPHICS_HPP_


#include <ksn/stuff.hpp>
#include <ksn/image.hpp>
#include <ksn/window_gl.hpp>
#include <ksn/math_vec.hpp>

#include "opencl.hpp"
#include "gl_switch.hpp"

#include <GL/glew.h>





struct texture_t
{
	cl::Buffer m_videodata;
	uint16_t m_width = 0, m_height = 0;

	//Loads the texture data and copies it into the video memory
	//Failure is a no-op unless the exception is not handled in the current stack frame and the destructor is called
	ksn::image_bgra_t::load_result_t load(const char* path);
};



struct object_t
{
	const texture_t* m_texture = nullptr;

	struct sprite_data_t
	{
		ksn::vec<2, uint16_t> m_texture_size; 
		ksn::vec<2, uint16_t> m_sprite_size;
		ksn::vec<2, uint16_t> m_sprite_texture_offset;
	} m_sprite_space_data{};

	struct transform_data_t
	{
		ksn::vec2f m_position;
		ksn::vec2f m_position_origin;
		ksn::vec2f m_rotation_origin;
		ksn::vec2f m_rotation_data = { 0, 1 }; //sin, cos
	} m_transform_data{};



	void set_sprite(const texture_t*, ksn::vec<2, uint16_t> position, ksn::vec<2, uint16_t> size);


	//Sets the rotaion angle
	void set_rotation(float angle) noexcept;
	void set_rotation_degrees(float angle) noexcept;

	//Adds the specified angle to the current one
	void rotate(float delta_angle) noexcept;
	void rotate_degrees(float delta_angle) noexcept;

	//Returns rotation angle
	float get_rotation() const noexcept;
	float get_rotation_degrees() const noexcept;


	void (*p_update_callback)(object_t&, float dt) = nullptr;
	//void (*p_update_postdraw_callback)(object_t&) = nullptr;
};


static_assert(sizeof(object_t::transform_data_t) == 32);
static_assert(sizeof(object_t::sprite_data_t) == 12);





struct draw_adapter_t
{
	std::vector<ksn::color_bgr_t> m_screen_data;

	cl::Buffer m_screen_videodata;
	cl::Buffer m_screen_videodata_downscaled;

#if DIGILOG_USE_OPENGL
	cl::Image2DGL m_render_buffer_cl;
	GLuint m_framebuffer = -1, m_render_buffer_gl = -1;
#else
	cl::Buffer m_screen_videodata_secondary;
	cl::Buffer m_screen_videodata_downscaled_secondary;
	bool m_do_display = false;
	void* m_mapped_ptr = nullptr;
#endif

	cl::Buffer* p1 = nullptr, * p2 = nullptr;
	cl::Buffer* p1d = nullptr, * p2d = nullptr;

	ksn::vec<2, uint16_t> m_size{ 0, 0 };
	uint8_t m_scaling = 1;



	~draw_adapter_t() noexcept;
	draw_adapter_t() noexcept;
	draw_adapter_t(const draw_adapter_t&) = delete;
	draw_adapter_t(draw_adapter_t&&) noexcept;



	void resize(uint16_t width, uint16_t height);

	void set_image_scaling(uint8_t n);



	void draw(const object_t&) const;

	void clear(ksn::color_bgra_t);

	void display();




private:
	
	void update_video_buffers();

	static void drawing_thread_worker(draw_adapter_t*) noexcept;

};

extern draw_adapter_t draw_adapter;



#endif //!_DIGILOG_GRAPHICS_HPP_
