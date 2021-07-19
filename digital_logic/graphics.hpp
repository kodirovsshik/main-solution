
#ifndef _DIGILOG_GRAPHICS_HPP_
#define _DIGILOG_GRAPHICS_HPP_


#include <ksn/stuff.hpp>
#include <ksn/image.hpp>
#include <ksn/window.hpp>

#include "opencl.hpp"





template<size_t>
struct image_overlay_type : std::false_type {};

template<>
struct image_overlay_type<0> : std::true_type {};
static constexpr image_overlay_type<0> image_overlay_replace;

template<>
struct image_overlay_type<1> : std::true_type {};
static constexpr image_overlay_type<1> image_overlay_default;





struct texture_t
{
	cl::Buffer m_videodata;
	ksn::image_bgra_t m_image;
	//bool m_sent = false;
};


struct sprite_t
{
	texture_t* m_texture;
	uint32_t m_width, m_height;
	int32_t m_x, m_y;
};



struct draw_adapter_t
{
	std::vector< ksn::color_bgra_t> m_screen_data;
	cl::Buffer m_screen_videodata;
	cl::Buffer m_screen_videodata_downscaled;
	uint32_t m_capacity = 0;
	uint8_t m_scaling = 1;



	draw_adapter_t() noexcept;
	draw_adapter_t(const draw_adapter_t&) = delete;
	draw_adapter_t(draw_adapter_t&&) noexcept;



	void resize(uint16_t width, uint16_t height);

	void set_image_scaling(uint8_t n);



	void draw_sprite() const noexcept;

	void clear(ksn::color_bgr_t) const noexcept;

	void display(ksn::window_t&) const noexcept;




private:
	
	void update_video_buffers();

};

extern draw_adapter_t draw_adapter;



#endif //!_DIGILOG_GRAPHICS_HPP_
