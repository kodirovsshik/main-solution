
#include "texture_2d.hpp"

namespace renderer
{
	namespace
	{
		namespace detail
		{
			GLuint _gen_textures(const ksn::image_bgra_t& img, GLenum filter, GLenum wrap)
			{
				GLuint tex;
				glGenTextures(1, &tex);
				glBindTexture(GL_TEXTURE_2D, tex);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, img.width, img.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.m_data.data());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrap);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			}
		}
	}

	texture2d_t::texture2d_t(const ksn::image_t& image, GLenum filter = GL_LINEAR, GLenum wrap = GL_CLAMP_TO_EDGE) noexcept
		: m_width(image.width), m_height(image.height)
	{
		this->m_id = detail::_gen_textures(image, filter, wrap);
	}


	texture2d_t& texture2d_t::operator=(texture2d_t&& other) noexcept
	{
		std::swap(this->m_id, other.m_id);
		std::swap(this->m_width, other.m_width);
		std::swap(this->m_height, other.m_height);
	}

	texture2d_t::~texture2d_t() noexcept
	{

	}


	void texture2d_t::bind() const noexcept
	{

	}
}