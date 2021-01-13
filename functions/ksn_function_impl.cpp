#include <ksn/function.hpp>
#include <ksn/instruction_set_x86-64.hpp>

#include <intrin.h>





_KSN_BEGIN



size_t c16len(const char16_t* p)
{
	const char16_t* pbegin = p;
	while (*p)
	{
		++p;
	}

	return p - pbegin;
}

size_t c32len(const char32_t* p)
{
	const char32_t* pbegin = p;
	while (*p)
	{
		++p;
	}

	return p - pbegin;
}





uint64_t random()
{
	uint16_t result[4];
	result[0] = ::rand();
	result[1] = ::rand();
	result[2] = ::rand();
	result[3] = ::rand();

	uint16_t temp = ::rand();

	result[0] |= ((temp << 15) & 0x8000);
	result[1] |= ((temp << 14) & 0x8000);
	result[2] |= ((temp << 13) & 0x8000);
	result[3] |= ((temp << 12) & 0x8000);

	return *(uint64_t*)result;
}

int64_t random(int64_t from, int64_t to)
{
	return random() % (to - from) + from;
}





void memory_dump(const void* _p, size_t bytes, size_t bytes_per_line, uint8_t flags, FILE* fd)
{
	uint8_t* p = (uint8_t*)_p, * pe = p + bytes;
	size_t byte_in_line = 0;
	char hex_a = flags & 1 ? 'a' : 'A';
	char buffer[4] = { 0 };
	size_t len = flags & 2 ? 2 : 3;

	while (p != pe)
	{
		uint8_t temp = *p >> 4;
		if (temp >= 10)
		{
			buffer[0] = (hex_a - 10) + temp;
		}
		else
		{
			buffer[0] = temp + '0';
		}

		temp = *p & 0xF;
		if (temp >= 10)
		{
			buffer[1] = (hex_a - 10) + temp;
		}
		else
		{
			buffer[1] = temp + '0';
		}

		if (++byte_in_line == bytes_per_line)
		{
			byte_in_line = 0;
			buffer[2] = '\n';
			fwrite(buffer, sizeof(char), 3, fd);
			buffer[2] = ' ';
		}
		else
		{
			fwrite(buffer, sizeof(char), len, fd);
		}

		++p;
	}
}





const void* memnotchr(const void* block, uint8_t value, size_t length)
{
	if (x86_features->avx512_bw)
	{
		//_mm512_cmpeq_epi8_mask gives 1 on equeal
		size_t local_size = length / 64;
		length &= 63;

		
	}
	else if (x86_features->avx2)
	{
		//_mm256_cmpeq_epi8 gives -1 on equal
		size_t local_size = length / 32;
		local_size &= 31;
	}
	else if (x86_features->sse2)
	{
		//_mm_cmpeq_epi8
		size_t local_size = length / 16;
		local_size &= 15;
	}
	else
	{

	}

	throw 0;
}





_KSN_END