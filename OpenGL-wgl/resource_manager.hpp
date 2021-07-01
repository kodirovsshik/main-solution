
#ifndef _RESOURCE_MANAGER_HPP_
#define _RESOURCE_MANAGER_HPP_


#include "shader_program.hpp"

#include <memory>
#include <unordered_map>


namespace resources
{
	class resource_manager_t
	{
	public:

		using pshader = renderer::shader_program*;
		using upshader = std::unique_ptr<renderer::shader_program>;

		pshader shader_load(const std::string_view& name, const std::string_view& path_vertex, const std::string_view& path_fragment) noexcept;
		pshader shader_get(const std::string_view& name) const noexcept;

		void free() noexcept;
		void free(const std::string_view& name) noexcept;

	private:

		std::unordered_map<std::string_view, upshader> m_shaders;

	};
}



#endif //!_RESOURCE_MANAGER_HPP_
