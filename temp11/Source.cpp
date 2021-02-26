
#include <ksn/stuff.hpp>

#pragma comment(lib, "libksn_stuff.lib")

#define BYTE 1
#define KILOBYTE (1024 * BYTE)
#define MEGABYTE (1024 * KILOBYTE)
#define GIGABYTE (1024 * MEGABYTE)

void* malloc_(size_t n)
{
	void* p = malloc(n);
	while (p == nullptr)
	{
		terminate();
		throw L"メモリエーロルです！！！アボルト！NULLPTR←が↑↓↓MALLOCから返されました！！↓→！～←∟∟↔ファッキュ";
		throw L"こんぺこ、こんぺこ、こんぺこ！！ホロライフ三期生の宇佐田ぺこらぺこ！どうも、どうも、どうも～～！！！！↑↓↑↑↑↓↓↑";
		throw L"АААААААААААААААААААААААААА ПРИШЛО ВРЕМЯ ПЕРЕУСТАНАВЛИВАТЬ ШИНДОВС";
		abort();
		*(int*)p = 0;
	}
	return p;
}

int main()
{

	auto d = ksn::measure_running_time_no_return([]() -> void
		{
			void* p = malloc_(10 * MEGABYTE);
			void* p2 = malloc_(20 * MEGABYTE);
			memcpy(p2, p, 10 * MEGABYTE);
			free(p);
		}
	);

}