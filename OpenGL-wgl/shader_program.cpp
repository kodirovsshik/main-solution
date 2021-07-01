
#include "shader_program.hpp"



namespace renderer
{
	namespace 
	{
		static constexpr size_t COMPILE_INFO_BUFSIZ = 2048;
		thread_local char error_info[COMPILE_INFO_BUFSIZ];

		GLuint create_shader(const char* src, int type, int len = -1)
		{
			if (len == -1) len = (int)strlen(src);
			GLuint shader = glCreateShader(type);
			glShaderSource(shader, 1, &src, &len);
			glCompileShader(shader);

			GLint ok = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
			if (!ok)
			{
				int len;
				glGetShaderInfoLog(shader, (GLsizei)COMPILE_INFO_BUFSIZ, &len, error_info);
				fprintf(stderr, "\n\aShader comilation error:\n%.*s\n", len, error_info);
				glDeleteShader(shader);
				shader = 0;
			}
			return shader;
		}

		GLuint create_program(const char* src_fragment, const char* src_vertex, size_t len_fragment = -1, size_t len_vertex = -1)
		{
			if (len_fragment == -1) len_fragment = strlen(src_fragment);
			if (len_vertex == -1) len_vertex = strlen(src_vertex);

			GLint ok = 0;
			GLuint shader_fragment = create_shader(src_fragment, GL_FRAGMENT_SHADER, (int)len_fragment);
			GLuint shader_vertex = create_shader(src_vertex, GL_VERTEX_SHADER, (int)len_vertex);

			if (shader_fragment == 0 || shader_vertex == 0)
			{
				if (shader_fragment) glDeleteShader(shader_fragment);
				if (shader_vertex) glDeleteShader(shader_vertex);
				return 0;
			}

			GLuint program = glCreateProgram();
			glAttachShader(program, shader_fragment);
			glAttachShader(program, shader_vertex);
			glLinkProgram(program);

			glDeleteShader(shader_fragment);
			glDeleteShader(shader_vertex);

			glGetProgramiv(program, GL_LINK_STATUS, &ok);
			if (!ok)
			{
				int len;
				glGetProgramInfoLog(program, (GLsizei)COMPILE_INFO_BUFSIZ, &len, error_info);
				fprintf(stderr, "\n\aShader program comilation error:\n%.*s\n", len, error_info);
				glDeleteProgram(program);
				program = -1;
			}
			return program;
		}
	}


	shader_program::shader_program() noexcept
		: m_program(-1), m_valid(false) {}

	shader_program::shader_program(const std::string_view& vs, const std::string_view& fs) noexcept
	{
		this->shader_program::shader_program();
		this->create(vs, fs);
	}
	shader_program::shader_program(shader_program&& x) noexcept
		: m_program(x.m_program), m_valid(x.m_valid) {}
	shader_program::~shader_program() noexcept
	{
		this->erase();
	}

	bool shader_program::create(const std::string_view& vs, const std::string_view& fs) noexcept
	{
		this->m_program = create_program(fs.data(), vs.data(), fs.length(), vs.length());
		return this->m_valid = (this->m_program != 0);
	}
	void shader_program::erase() noexcept
	{
		if (this->m_valid) glDeleteProgram(this->m_program);
		this->m_valid = false;
		this->m_program = -1;
	}

	shader_program::operator bool() const noexcept
	{
		return this->m_valid;
	}

	void shader_program::use() const noexcept
	{
		if (this->m_valid) glUseProgram(this->m_program);
	}

	shader_program& shader_program::operator=(shader_program&& x) noexcept
	{
		std::swap(this->m_program, x.m_program);
		std::swap(this->m_valid, x.m_valid);
		return *this;
	}

} //namespace renderer
