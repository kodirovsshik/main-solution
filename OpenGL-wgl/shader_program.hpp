
#ifndef _SHADER_PROGRAM_HPP_
#define _SHADER_PROGRAM_HPP_



#include <GL/glew.h>

#include <string>



namespace renderer
{
	class shader_program
	{
	private:

		GLuint m_program;
		bool m_valid;


	public:

		shader_program() noexcept;
		shader_program(const shader_program&) = delete;
		shader_program(shader_program&&) noexcept;
		shader_program(const std::string_view& vertex_source, const std::string_view& fragment_source) noexcept;
		~shader_program() noexcept;

		shader_program& operator=(const shader_program&) = delete;
		shader_program& operator=(shader_program&&) noexcept;

		bool create(const std::string_view& vertex_source, const std::string_view& fragment_source) noexcept;
		void erase() noexcept;

		explicit operator bool() const noexcept;

		void use() const noexcept;
	};
}

#endif //!_SHADER_PROGRAM_HPP_
