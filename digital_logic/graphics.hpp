
#ifndef _DIGILOG_GRAPHICS_HPP_
#define _DIGILOG_GRAPHICS_HPP_


#include <ksn/stuff.hpp>
#include <ksn/image.hpp>
#include <ksn/window.hpp>
#include <ksn/math_vec.hpp>

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
	uint16_t m_width, m_height;

	ksn::image_bgra_t::load_result_t load(const char* path);
};



struct object_t
{
	texture_t* m_texture = nullptr;

	struct sprite_data_t
	{
		ksn::vec<2, uint16_t> m_texture_size; 
		ksn::vec<2, uint16_t> m_sprite_size;
		ksn::vec<2, uint16_t> m_sprite_texture_offset;
	} m_sprite_space_data{};

	struct transform_data_t
	{
		ksn::vec<2, int32_t> m_position;
		ksn::vec<2, uint16_t> m_position_origin;
		ksn::vec<2, uint16_t> m_rotation_origin;
		float m_rotation;
	} m_transform_data{};


	void set_sprite(texture_t*, ksn::vec<2, uint16_t> position, ksn::vec<2, uint16_t> size);
};


static_assert(sizeof(object_t::transform_data_t) == 20);
static_assert(sizeof(object_t::sprite_data_t) == 12);





struct draw_adapter_t
{
	std::vector<ksn::color_bgra_t> m_screen_data;
	cl::Buffer m_screen_videodata;
	cl::Buffer m_screen_videodata_downscaled;
	uint16_t m_width = 0, m_height = 0;
	uint8_t m_scaling = 1;



	draw_adapter_t() noexcept;
	draw_adapter_t(const draw_adapter_t&) = delete;
	draw_adapter_t(draw_adapter_t&&) noexcept;



	void resize(uint16_t width, uint16_t height);

	void set_image_scaling(uint8_t n);



	void draw(const object_t&) const;

	void clear(ksn::color_bgra_t);

	void display(ksn::window_t&);




private:
	
	void update_video_buffers();

};

extern draw_adapter_t draw_adapter;



#endif //!_DIGILOG_GRAPHICS_HPP_
