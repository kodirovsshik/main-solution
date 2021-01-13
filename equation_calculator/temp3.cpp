#include <stdio.h>
#include <stdarg.h>

#include <ksn/math.hpp>

#include <set>
#include <iostream>

#pragma comment(lib, "ksn_math.lib")

#pragma warning(disable : 6031)

bool extract_const(const ksn::formula& f, double& d)
{
	if (f.m_p8_equation == nullptr || f.m_length < 9)
	{
		return false;
	}

	uint8_t* p = f.m_p8_equation;
	if (*p != _KSN_MATH_FORMULA_INSTRUCTION_RET_C)
	{
		return false;
	}

	double* pd = (double*)(p + 1);
	d = *pd;
	return true;
}

double parse_expression(const char* p, double default_value = NAN)
{
	double result = default_value;

	ksn::formula f;
	if (f.parse(p) != 1)
	{
		return result;
	}

	extract_const(f, result);
	return result;
}

double fix_double(double arg)
{
	union
	{
		double value;
		uint8_t byte[8];
	} a;

	union
	{
		uint64_t value;
		uint8_t byte[8];
	} b;

	a.value = arg;

	for (size_t i = 0; i < 8; ++i)
	{
		b.byte[i] = a.byte[7 - i];
	}

	b.value >>= 9;
	b.value <<= 1;

	return 0;
}

#pragma warning(disable : 4996)

bool fake_input(const void* data, size_t len = -1, FILE* fd = stdin)
{
	char** current;
	int* b_len;
	_get_stream_buffer_pointers(fd, nullptr, &current, &b_len);
	
	if (!current || !b_len)
		return false;

	if (len == -1)
		len = strlen((char*)data);

	memcpy(*current, data, len);
	*b_len += len;

	return true;
}

int main()
{
	ksn::formula f;
	std::string s;
	ksn::formula::g_perform_bracket_mismatch_check = true;

	//char buff[512];
	//setvbuf(stdin, buff, _IOFBF, 512);

	//fake_input("x^3 - 2x^2 - x + 2\n");

	printf("y(x)=");

	while (1)
	{
		std::string s;
		std::getline(std::cin, s);

		int res = f.parse(s.data());
		if (res != 1)
		{
			printf("Failed to parse formula, err at position %i, try again\ny(x)=", -(int)res);
		}
		else
		{
			break;
		}
	}

	if (f.m_variables_numbers.size() == 0)
	{
		printf("y=%lg\n", f({}));
		return 0;
	}

	printf("Order of arguments: ");
	for (const auto& v : f.m_variables_numbers)
	{
		printf("%s ", v.first.operator std::string().data());
	}

	while (1) 
	{
		std::vector<double> args;

		printf("\nEnter arguments separated by space: ");
		for (size_t i = 0; i < f.m_variables_numbers.size(); ++i)
		{
			double x;
			while (scanf("%lg", &x) != 1)
			{
				printf("Failed to read %zu", i);
				if ((i % 100) / 10 == 1)
				{
					fwrite("th", sizeof(char), 2, stdout);
				}
				else
				{
					switch (i % 10)
					{
					case 1:
						fwrite("st", sizeof(char), 2, stdout);
						break;

					case 2:
						fwrite("nd", sizeof(char), 2, stdout);
						break;

					default:
						fwrite("th", sizeof(char), 2, stdout);
					}
				}

				fwrite("(from 0) argument, try again: ", sizeof(char), 30, stdout);

				rewind(stdin);
				fflush(stdin);
			}
			args.push_back(x);
		}

		printf("y(args)=%lg\n", f(args));
	}
}
