
#include <ksn/crc.hpp>
#include <ksn/stuff.hpp>

#include <stdio.h>

#include <string>


#pragma warning(disable : 4996)
#pragma comment(lib, "libksn_crc")


int main()
{
	
	const std::tuple<std::string, std::string> names[] =
	{
		{"shader_vertex", "vertex"},
		{"shader_fragment", "fragment"},
	};

	const std::string path_prefix = "D:/Projects/C++/Solution/VulkanStudy/resources/";
	const std::string compiler_options = " -O --target-env=vulkan1.2 ";

	for (const auto& [name, stage] : names)
	{
		std::string name_in = path_prefix + name + ".glsl";
		std::string name_out = path_prefix + name + ".spr";
		std::string name_crc = path_prefix + name + ".crc64";
		std::string command = "glslc -fshader-stage=" + stage + " \"" + name_in + "\" -o \"" + name_out + "\" " + compiler_options;
		if (system(command.c_str()) != 0)
		{
			printf("Failed to execute:\n%s\n", command.c_str());
			break;
		}

		FILE* fin = fopen(name_out.c_str(), "rb");
		FILE* fout = fopen(name_crc.c_str(), "wb");
		if (!fin)
		{
			printf("Failed to open: %s\n", name_out.c_str());
			break;
		}
		if (!fout)
		{
			printf("Failed to open: %s\n", name_crc.c_str());
			break;
		}

		uint64_t crc = ksn::crc64_ecma_init();
		char buffer[4096];
		while (!feof(fin))
		{
			size_t read = fread(buffer, sizeof(char), ksn::countof(buffer), fin);
			crc = ksn::crc64_ecma_update(buffer, read, crc);
		}
		fprintf(fout, "%llX", crc);

		fclose(fin);
		fclose(fout);
	}

	return 0;
}
