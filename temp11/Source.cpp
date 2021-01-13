#include <filesystem>
#include <algorithm>

int main1(int, char** argv)
{
	std::string path = argv[0];
	path[path.find_last_of('\\') + 1] = 0;
	while (1)
	{
		size_t pos = path.find('\\');
		if (pos == std::string::npos) break;
		path[pos] = '/';
	}

	printf("%s\n", path.c_str());
	for (const auto& dir : std::filesystem::directory_iterator(".."))
	{
		if (dir.is_directory() == false) continue;
		
		auto dirname = dir.path().filename().string();
		if (dirname.substr(dirname.find_last_of('\\') + 1) == "temp11") continue;

		for (const auto& subdir : std::filesystem::directory_iterator(dir.path()))
		{
			auto name = subdir.path().string();
			name = name.substr(name.find_last_of('\\') + 1);

			for (char& ch : name)
			{
				ch = std::tolower(ch);
			}
			if (name == "x64" || name == "debug" || name == "release")
			{
				try
				{
					std::filesystem::remove_all(subdir.path());
				}
				catch (const std::exception& e)
				{
					printf("%s", e.what());
				}
			}
		}
	}

	return 0;
}

int main(int argc, char** argv)
{
	setlocale(0, "");
	try
	{
		return main1(argc, argv);
	}
	catch (const std::exception& e)
	{
		printf("%s", e.what());
	}
	return -1;
}