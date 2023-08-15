
import <Windows.h>;
import std;


using namespace std;
using namespace filesystem;

const size_t N = 1024 * 1024;
char buff1[N], buff2[N];

void safe_open(ifstream& f, const path& p)
{
	f.open(p, ios::binary | ios::in);
	if (!f.is_open())
		throw filesystem_error("Failed to open file for reading", p, std::error_code());
}

namespace filecmp
{
	static bool equal(const path& p1, const path& p2)
	{
		ifstream f1, f2;
		safe_open(f1, p1);
		safe_open(f2, p2);

		while (true)
		{
			f1.read(buff1, N);
			auto s1 = f1.gcount();

			f2.read(buff2, N);
			auto s2 = f2.gcount();

			if (s1 != s2)
				return false;

			if (memcmp(buff1, buff2, s1) != 0)
				return false;

			if (f1.eof() && f2.eof())
				break;

			if (f1.eof() != f2.eof())
				return false;
		}

		return true;
	}

	static bool differ(const path& p1, const path& p2)
	{
		return !equal(p1, p2);
	}
};

template<class... Args>
[[noreturn]] void stop(const char* fmt, Args&& ...args)
{
	vprint_nonunicode(fmt, std::make_format_args(std::forward<Args>(args)...));
	cin.get();
	exit(1);
}

auto collect_directory(const path& p)
{
	vector<directory_entry> result;

	auto dir = ranges::subrange(directory_iterator(p), directory_iterator());
	for (const auto& entry : dir)
		result.push_back(entry);

	ranges::sort(result, {}, [](const auto& entry) { return entry.path(); });
	return result;
}

void traverse_directory(const path& dpath1, const path& dpath2)
{
	auto d1 = collect_directory(dpath1);
	auto d2 = collect_directory(dpath2);
	
	auto p1 = d1.begin();
	auto p2 = d2.begin();

	const auto p1e = d1.end();
	const auto p2e = d2.end();

	while (p1 != p1e && p2 != p2e)
	{
		const auto path1 = p1->path();
		const auto path2 = p2->path();

		const auto cmp = path1.filename() <=> path2.filename();
		if (cmp < 0)
		{
			println("Structure mismatch: no match for {}", path1.string());
			++p1;
		}
		else if (cmp > 0)
		{
			println("Structure mismatch: no match for {}", path2.string());
			++p2;
		}
		else
		{
			const bool dir1 = p1->is_directory(), dir2 = p2->is_directory();
			if (dir1 && dir2)
				traverse_directory(path1, path2);
			else if (dir1 != dir2)
				println("Structure mismatch: {} vs {}", path1.string(), path2.string());
			else if (filecmp::differ(path1, path2))
				println("Data mismatch: {} vs {}", path1.string(), path2.string());
			++p1;
			++p2;
		}
	}

	while (p1 != p1e)
	{
		const auto path1 = p1->path();
		println("Structure mismatch: no match for {}", path1.string());
		++p1;
	}
	while (p2 != p2e)
	{
		const auto path2 = p2->path();
		println("Structure mismatch: no match for {}", path2.string());
		++p2;
	}
}

int main(int argc, const char* argv[])
{
	auto loc = setlocale(0, "");

#if 1
	const int _argc = 3;
	const char* _argv[_argc] = { argv[0], "D:/Projects/C++/Solution/x64/Debug", "D:/Projects/C++/Solution/x64/"};
	argc = _argc;
	argv = _argv;
#endif

	if (argc != 3)
		stop("Two paths expected");

	try
	{
		path p1(argv[1]);
		path p2(argv[2]);

		traverse_directory(p1, p2);

		print("done");
	}
	catch (const filesystem_error& e)
	{
		stop("filesystem error: {}\np1: {}", e.what(), e.path1().string());
	}
}
