
#include "resource_manager.hpp"

#include <filesystem>



#pragma warning(disable : 4996)



namespace
{
	template<typename byte_t> requires(sizeof(byte_t) == 1)
	bool read_file_contents(const std::string_view& file, std::vector<byte_t>& result, const char* open_mode) noexcept
	{
		try
		{
			FILE* fd = fopen(std::string(file).c_str(), open_mode);
			if (!fd) return false;

			size_t size = std::filesystem::file_size(file);

			result.resize(size + 1);
			size = fread(result.data(), 1, size, fd);
			result.resize(size + 1);
			result.back() = 0;

			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	template<typename byte_t> requires(sizeof(byte_t) == 1)
	bool read_file_binary_contents(const std::string_view& file, std::vector<byte_t>& result) noexcept
	{
		return read_file_contents(file, result, "rb");
	}

	template<typename byte_t> requires(sizeof(byte_t) == 1)
	bool read_file_text_contents(const std::string_view& file, std::vector<byte_t>& result) noexcept
	{
		return read_file_contents(file, result, "r");
	}

}



namespace resources
{
	resource_manager_t::pshader resource_manager_t::shader_load(const std::string_view& name, const std::string_view& path_vertex, const std::string_view& path_fragment) noexcept
	{
		if (this->m_shaders.count(name))
			this->m_shaders.erase(name);

		std::vector<char> src_vertex;
		std::vector<char> src_fragment;

		if (!read_file_text_contents(path_vertex, src_vertex)) return nullptr;
		if (!read_file_text_contents(path_fragment, src_fragment)) return nullptr;

		auto shader_program = std::make_unique<renderer::shader_program>(std::string_view{ src_vertex.data(), src_vertex.size() }, std::string_view{ src_fragment.data(), src_fragment.size() });
		auto* result = shader_program.get();

		this->m_shaders.insert({ name, std::move(shader_program) });
		return result;
	}
	resource_manager_t::pshader resource_manager_t::shader_get(const std::string_view& name) const noexcept
	{
		if (this->m_shaders.count(name))
			return this->m_shaders.at(name).get();
		else
			return nullptr;
	}

	void resource_manager_t::free() noexcept
	{
		this->m_shaders.clear();
	}
	void resource_manager_t::free(const std::string_view& name) noexcept
	{
		this->m_shaders.erase(name);
	}
}
