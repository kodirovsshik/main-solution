
#ifndef _TEXTURE_2D_HPP_
#define _TEXTURE_2D_HPP_

#include <ksn/image.hpp>

#include <GL/glew.h>



namespace renderer
{
	class texture2d_t
	{
	public:

		texture2d_t() = delete;
		texture2d_t(const texture2d_t&) = delete;
		texture2d_t(texture2d_t&&) noexcept;
		texture2d_t(const ksn::image_t& image, GLenum filter = GL_LINEAR, GLenum wrap = GL_CLAMP_TO_EDGE) noexcept;

		texture2d_t& operator=(const texture2d_t&) = delete;
		texture2d_t& operator=(texture2d_t&&) noexcept;

		~texture2d_t() noexcept;


		void bind() const noexcept;

	private:
		GLuint m_id;
		uint32_t m_width, m_height;
	};
}



#endif //!_TEXTURE_2D_HPP_
