// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <ksn/ksnio.hpp>

#include <string> //char_traits<T>::length

#pragma warning(disable : 6011)


#ifdef _DEBUG
#define empty() {int a = 0;}((void)0)
#else
#define empty() ((void)0)
#endif


_KSN_BEGIN


template<class CharT>
class ksnio_output_buffer
{

	void (ksnio_output_buffer<CharT>::*append8)(const char*, size_t);
	void (ksnio_output_buffer<CharT>::*append16)(const char16_t*, size_t);
	void (ksnio_output_buffer<CharT>::*append32)(const char32_t*, size_t);

public:
	ksnio_output_buffer()
	{
		
	}

	template<typename T>
	void append(const T* buffer, size_t len)
	{
		
	}

	template<>
	void append(const char* buffer, size_t len)
	{
		this->*append8(buffer, len);
	}
	template<>
	void append(const wchar_t* buffer, size_t len)
	{
		switch (sizeof(wchar_t))
		{
		case sizeof(char) :
			return this->*append8((char*)buffer, len);
		case sizeof(char16_t) :
			return this->*append16((char16_t*)buffer, len);
		case sizeof(char32_t) :
			return this->*append32((char32_t*)buffer, len);
		}
	}
	template<>
	void append(const char16_t* buffer, size_t len)
	{
		this->*append16(buffer, len);
	}
	template<>
	void append(const char32_t* buffer, size_t len)
	{
		this->*append32(buffer, len); 
	}

	void append(CharT symbol, size_t times = 1)
	{

	}

	~ksnio_output_buffer()
	{

	}
};



#define arg_int8 1
#define arg_int16 2
#define arg_int32 4
#define arg_int64 8
#define arg_char 16
#define arg_cstring 32
#define arg_n 64
#define arg_unsigned 128
#define arg_uppercase 256
#define arg_hexadecimal 512
#define arg_octal 1024
#define arg_binary 2048
#define arg_pointer 4096
#define arg_float 8192
#define arg_fp_fixed 16384
#define arg_fp_exponential 32768

using arg_t = uint16_t;



template<class CharT>
arg_t parse_argument_type(const CharT* & format)
{
	--format;
	switch (*format++)
	{
	case 'c':
		return arg_char;
	case 's':
		return arg_cstring | arg_int8;
	case 'i':
		return sizeof(int);
	case 'u':
		return sizeof(int) | arg_unsigned;
	case 'x':
		return sizeof(int) | arg_hexadecimal | arg_unsigned;
	case 'X':
		return sizeof(int) | arg_hexadecimal | arg_uppercase | arg_unsigned;
	case 'b':
		return sizeof(int) | arg_binary | arg_unsigned;
	case 'o':
		return sizeof(int) | arg_octal | arg_unsigned;
	case 'O':
		return sizeof(int) | arg_octal | arg_uppercase | arg_unsigned;
	case 'n':
		return sizeof(int) | arg_n;
	case 'h':
		switch (*format++)
		{
		case 'i':
			return sizeof(short);
		case 'u':
			return sizeof(short) | arg_unsigned;
		case 'x':
			return sizeof(short) | arg_hexadecimal | arg_unsigned;
		case 'X':
			return sizeof(short) | arg_hexadecimal | arg_uppercase | arg_unsigned;
		case 'b':
			return sizeof(short) | arg_binary | arg_unsigned;
		case 'o':
			return sizeof(short) | arg_octal | arg_unsigned;
		case 'O':
			return sizeof(short) | arg_octal | arg_uppercase | arg_unsigned;
		case 'n':
			return sizeof(short) | arg_n;
		case 'h':
			switch (*format++)
			{
			case 'i':
				return sizeof(char);
			case 'u':
				return sizeof(char) | arg_unsigned;
			case 'x':
				return sizeof(char) | arg_hexadecimal | arg_unsigned;
			case 'X':
				return sizeof(char) | arg_hexadecimal | arg_uppercase | arg_unsigned;
			case 'b':
				return sizeof(char) | arg_binary | arg_unsigned;
			case 'o':
				return sizeof(char) | arg_octal | arg_unsigned;
			case 'O':
				return sizeof(char) | arg_octal | arg_uppercase | arg_unsigned;
			case 'n':
				return sizeof(char) | arg_n;
			default:
				return 0;
			}
		default:
			return 0;
		}
	case 'l':
		switch (*format++)
		{
		case 'c':
			return sizeof(wchar_t) | arg_char;
		case 's':
			return sizeof(wchar_t) | arg_cstring;
		case 'i':
			return sizeof(long);
		case 'u':
			return sizeof(long) | arg_unsigned;
		case 'x':
			return sizeof(long) | arg_hexadecimal | arg_unsigned;
		case 'X':
			return sizeof(long) | arg_hexadecimal | arg_uppercase | arg_unsigned;
		case 'b':
			return sizeof(long) | arg_binary | arg_unsigned;
		case 'o':
			return sizeof(long) | arg_octal | arg_unsigned;
		case 'O':
			return sizeof(long) | arg_octal | arg_uppercase | arg_unsigned;
		case 'n':
			return sizeof(long) | arg_n;
		case 'l':
			switch (*format++)
			{
			case 'i':
				return sizeof(long long);
			case 'u':
				return sizeof(long long) | arg_unsigned | arg_unsigned;
			case 'x':
				return sizeof(long long) | arg_hexadecimal | arg_unsigned;
			case 'X':
				return sizeof(long long) | arg_hexadecimal | arg_uppercase | arg_unsigned;
			case 'b':
				return sizeof(long long) | arg_binary | arg_unsigned;
			case 'o':
				return sizeof(long long) | arg_octal | arg_unsigned;
			case 'O':
				return sizeof(long long) | arg_octal | arg_uppercase | arg_unsigned;
			case 'n':
				return sizeof(long long) | arg_n;
			default:
				return 0;
			}
		case 'f':
			return arg_float | arg_fp_fixed | sizeof(long double);
		case 'e':
			return arg_float | arg_fp_exponential | sizeof(long double);
		case 'E':
			return arg_float | arg_fp_exponential | arg_uppercase | sizeof(long double);
		case 'g':
			return arg_float | sizeof(long double);
		case 'G':
			return arg_float | arg_uppercase | sizeof(long double);
		default:
			return 0;
		}
	case 'w':
		switch (*format++)
		{
		case 'c':
			return sizeof(wchar_t) | arg_char;
		case 's':
			return sizeof(wchar_t) | arg_char;
		default:
			return 0;
		}
	case 'L':
		switch (*format++)
		{
		case 'f':
			return arg_float | arg_fp_fixed | sizeof(long double);
		case 'e':
			return arg_float | arg_fp_exponential | sizeof(long double);
		case 'E':
			return arg_float | arg_fp_exponential | arg_uppercase | sizeof(long double);
		case 'g':
			return arg_float | sizeof(long double);
		case 'G':
			return arg_float | arg_uppercase | sizeof(long double);
		default:
			return 0;
		}
	case 'f':
		return arg_float | arg_fp_fixed;
	case 'e':
		return arg_float | arg_fp_exponential;
	case 'E':
		return arg_float | arg_fp_exponential | arg_uppercase;
	case 'g':
		return arg_float;
	case 'G':
		return arg_float | arg_uppercase;
	case 'p':
		return arg_pointer;
	case 'P':
		return arg_pointer | arg_uppercase;
	default:
		return 0;
	}
}





template<class CharT, class StringT>
static void ksnio_printf_helper_puts(ksnio_output_buffer<CharT>& buffer, bool have_precision, int precision, int width, CharT filler, const StringT* string)
{
	size_t len;
	if (have_precision)
	{
		//assume that const CharT *string may or may not be a null-terminated string
		//so, calculate length ourselves but don't read more than precision elements
		const StringT* p_string = string, *p_string_end = string + precision;
		while (p_string < p_string_end)
		{
			if (*p_string++ == '\0')
			{
				break;
			}
		}
		len = p_string - string;
	}
	else
		len = std::char_traits<StringT>::length(string);
	
	if (width < 0)
	{
		buffer.append(string, len);
		width = -width - len;
		if (width > 0)
		{
			buffer.append(filler, width);
		}
	}
	else
	{
		width -= len;
		if (width > 0)
		{
			buffer.append(filler, width);
		}
		buffer.append(string, len);
	}
}


#define next() (fmt = *format++)

template<class CharT>
static int ksnio_printf(ksnio_output_buffer<CharT>& buffer, const CharT* format, va_list ap)
{
	if (format == nullptr || ap == nullptr)
		return -1;

	//state vars
	CharT filler;
	int width;
	int precision;
	bool have_precision;

	//flags
	bool flag_sign;
	bool flag_pad_left;
	bool flag_prefix;
	bool flag_fill_zeroes;
	bool flag_blank_sign;
	bool flag_invert_prefix_case;

	auto ksnio_printf_helper_putnum = [&]
	(int64_t x, arg_t arg)
	{
		uint64_t ux = uint64_t(x);
		CharT cbuffer[70];
		CharT* p_buffer = cbuffer + 70;
		bool negative = false;

		if ((arg & arg_unsigned) == 0)
		{
			if (x == 0)
			{
				*--p_buffer = '0';
				goto number_postprocess;
			}

			if (x < 0)
			{
				*--p_buffer = -(x % 10) + '0';
				x /= -10;
				negative = true;
			}

			while (x)
			{
				*--p_buffer = x % 10 + '0';
				x /= 10;
			}
		}
		else if (x == 0)
		{
			*--p_buffer = '0';
		}
		else if (arg & arg_binary)
		{
			while (ux)
			{
				*--p_buffer = '0' + (ux & 1);
				ux >>= 1;
			}
		}
		else if (arg & arg_octal)
		{
			while (ux)
			{
				*--p_buffer = '0' + (ux & 7);
				ux >>= 3;
			}
		}
		else if (arg & arg_hexadecimal)
		{
			if (arg & arg_uppercase)
			{
				while (ux)
				{
					if ((ux & 15) < 10)
						* --p_buffer = '0' + (ux & 15);
					else
						*--p_buffer = 'A' + (ux & 15);
					ux >>= 4;
				}
			}
			else
			{
				while (ux)
				{
					if ((ux & 15) < 10)
						* --p_buffer = '0' + (ux & 15);
					else
						*--p_buffer = 'a' + (ux & 15);
					ux >>= 4;
				}
			}
		}
		else
		{
			while (ux)
			{
				*--p_buffer = x % 10 + '0';
				x /= 10;
			}
		}

	number_postprocess:
		size_t number_length = cbuffer + 70 - p_buffer;
		size_t expression_length = number_length;

		if (precision > 0 && size_t(precision) > expression_length)
		{
			expression_length = precision;
		}

		if (flag_prefix)
		{
			if (arg & (arg_binary | arg_hexadecimal))
			{
				expression_length += 2;
			}
			else if (arg & arg_octal)
			{
				expression_length++;
				if (arg & arg_uppercase)
				{
					expression_length++;
				}
			}
		}

		if (((arg & (arg_hexadecimal | arg_binary | arg_octal)) == 0))
		{
			if (negative)
			{
				//'-'
				expression_length++;
			}
			else if (flag_sign && (ux > 0 || x > 0))
			{
				//'+'
				expression_length++;
			}
			else if (flag_blank_sign)
			{
				//' '
				expression_length++;
			}
		}

		width -= expression_length;

		if (!flag_pad_left && width > 0)
		{
			buffer.append(filler, width);
		}

		if (((arg & (arg_hexadecimal | arg_binary | arg_octal)) == 0))
		{
			if (negative)
			{
				buffer.append('-');
			}
			else if (flag_sign && (ux > 0 || x > 0))
			{
				buffer.append('+');
			}
			else if (flag_blank_sign)
			{
				buffer.append(' ');
			}
		}

		if (flag_prefix)
		{
			CharT prefix[3] = { '0', 0, 0 };

			if (arg & arg_binary)
			{
				prefix[1] = (bool(arg & arg_uppercase) ^ flag_invert_prefix_case) ? 'b' : 'B';
			}
			else if (arg & arg_hexadecimal)
			{
				prefix[1] = (bool(arg & arg_uppercase) ^ flag_invert_prefix_case) ? 'b' : 'B';
			}
			else if (arg & arg_octal)
			{
				if (arg & arg_uppercase)
				{
					if (flag_invert_prefix_case)
					{
						prefix[1] = 'o';
					}
					else
					{
						prefix[1] = 'O';
					}
				}
			}

			buffer.append(prefix);
		}

		precision -= number_length;
		if (precision > 0)
		{
			buffer.append('0', precision);
		}

		buffer.append(p_buffer, number_length);

		if (flag_pad_left && width > 0)
		{
			buffer.append(filler, width);
		}
	};

	CharT fmt;
	
	while (next() != '\0')
	{
		//perform reset
		filler = ' ';
		width = 0;
		precision = 0;
		have_precision = false;

		flag_sign = false;
		flag_pad_left = false;
		flag_prefix = false;
		flag_fill_zeroes = false;
		flag_blank_sign = false;
		flag_invert_prefix_case = false;


		//Find next '%', put all other characters into buffer until '%' is found (or, of course, if '\0' is reached)
		if (fmt != '%')
		{
			const CharT* b_format = format - 1;

			while (*format != '%' && *format != '\0')
				++format;

			buffer.append(b_format, format - b_format);
			continue;
		}
		

		//check for "%%"
		if (next() == '%')
		{
			buffer.append('%');
			continue;
		}


		//flags
		while (1)
		{
			if (fmt == '+')
				flag_sign = true;
			else if (fmt == '-')
				flag_pad_left = true;
			else if (fmt == ' ')
				flag_blank_sign = true;
			else if (fmt == '#')
				flag_prefix = true;
			else if (fmt == '0')
				flag_fill_zeroes = true;
			else if (fmt == '^')
				flag_invert_prefix_case = true;
			else
				break;
			next();
		}


		//width
		if (fmt == '*')
		{
			width = va_arg(ap, int);
			next();
		}
		else
			while (isdigit(fmt))
			{
				width = (width << 3) + (width << 1) + (fmt - '0');
				next();
			}


		//pad symbol
		if (fmt == '.') { }
		else if (fmt == ',')
			filler = next();
		else if (fmt == '/')
			filler = va_arg(ap, CharT);


		//precision
		if (fmt == '*')
		{
			have_precision = true;
			precision = va_arg(ap, int);
			next();
		}
		else if (isdigit(fmt))
		{
			have_precision = true;
			while (isdigit(fmt))
			{
				precision = (precision << 3) + (precision << 1) + (fmt - '0');
				next();
			}
		}


		//argument type
		arg_t arg = parse_argument_type(format);


		//processing
		if (arg & arg_cstring)
		{
			int temp = flag_pad_left ? -width : width;
			if (arg & arg_int8)
				ksnio_printf_helper_puts<CharT, char>(buffer, have_precision, precision, temp, filler, va_arg(ap, const char*));
			else if (arg & arg_int16)
				ksnio_printf_helper_puts<CharT, char16_t>(buffer, have_precision, precision, temp, filler, va_arg(ap, const char16_t*));
			else if (arg & arg_int16)
				ksnio_printf_helper_puts<CharT, char32_t>(buffer, have_precision, precision, temp, filler, va_arg(ap, const char32_t*));
		}
		else if (arg & arg_pointer)
		{
			if (have_precision == false)
			{
				precision = sizeof(void*) * 2;
			}

			arg |= arg_hexadecimal;

			//tbh, idk what compiler will come up with if i'll ask it to convert void* to int64_t,
			//so i'd rather do it myself like this:
			//uintptr_t -> uint64_t -> int64_t
			//Yes, i could have omit the last convertion, but i'm pretty sure compiler will give a warning
			ksnio_printf_helper_putnum((int64_t)(uint64_t)va_arg(ap, uintptr_t*), arg);
		}
		else if (arg & arg_float)
		{
			long double val;
			if (arg & sizeof(long double))
				val = va_arg(ap, long double);
			else
				val = va_arg(ap, double);
		}
		else
		{
			int64_t x;
			
			//Number
			if (arg & arg_unsigned)
			{
				if (arg & arg_int8)
				{
					x = (int64_t)(uint64_t)va_arg(ap, uint8_t);
				}
				else if (arg & arg_int16)
				{
					x = (int64_t)(uint64_t)va_arg(ap, uint16_t);
				}
				else if (arg & arg_int32)
				{
					x = (int64_t)(uint64_t)va_arg(ap, uint32_t);
				}
				else if (arg & arg_int64)
				{
					x = (int64_t)(uint64_t)va_arg(ap, uint64_t);
				}
				else //???
					continue;
			}
			else if (arg & arg_int8)
			{
				x = va_arg(ap, int8_t);
			}
			else if (arg & arg_int16)
			{
				x = va_arg(ap, int16_t);
			}
			else if (arg & arg_int32)
			{
				x = va_arg(ap, int32_t);
			}
			else if (arg & arg_int64)
			{
				x = va_arg(ap, int64_t);
			}
			else //???
				continue;

			ksnio_printf_helper_putnum(x, arg);
		}
	}

	return buffer.get_total();
}

#undef next








int kprintf(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	int res = kvfprintf(stdout, format, ap);

	va_end(ap);
	return res;
}
int kfprintf(FILE*, const char* format, ...);
int kvprintf(const char* format, va_list);
int kvfprintf(FILE* fd, const char* format, va_list ap)
{
	return -1;
}

int ksprintf(char* buffer, const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	int res = kvsnprintf(buffer, -1, format, ap);

	va_end(ap);

	return res;
}
int kvsprintf(char* buffer, const char* format, va_list);
int ksnprintf(char* buffer, size_t bufferSize, const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	int res = kvsnprintf(buffer, bufferSize, format, ap);

	va_end(ap);

	return res;
}
int kvsnprintf(char* buffer, size_t bufferSize, const char* format, va_list ap)
{
	return -1;
}


int kwprintf(const wchar_t* format, ...);
int kfwprintf(FILE*, const wchar_t* format, ...);
int kvwprintf(const wchar_t* format, va_list);
int kvfwprintf(FILE*, const wchar_t* format, va_list);

int kswprintf(wchar_t* buffer, const wchar_t* format, ...);
int kvswprintf(wchar_t* buffer, const wchar_t* format, va_list);
int ksnwprintf(wchar_t* buffer, size_t bufferSize, const wchar_t* format, ...);
int kvsnwprintf(wchar_t* buffer, size_t bufferSize, const wchar_t* format, va_list);


int k16printf(const char16_t* format, ...);
int kf16printf(FILE*, const char16_t* format, ...);
int kv16printf(const char16_t* format, va_list);
int kvf16printf(FILE*, const char16_t* format, va_list);

int ks16printf(char16_t* buffer, const char16_t* format, ...);
int kvs16printf(char16_t* buffer, const char16_t* format, va_list);
int ksn16printf(char16_t* buffer, size_t bufferSize, const char16_t* format, ...);
int kvsn16printf(char16_t* buffer, size_t bufferSize, const char16_t* format, va_list);


int k32printf(const char32_t* format, ...);
int kf32printf(FILE*, const char32_t* format, ...);
int kv32printf(const char32_t* format, va_list);
int kvf32printf(FILE*, const char32_t* format, va_list);

int ks32printf(char32_t* buffer, const char32_t* format, ...);
int kvs32printf(char32_t* buffer, const char32_t* format, va_list);
int ksn32printf(char32_t* buffer, size_t bufferSize, const char32_t* format, ...);
int kvsn32printf(char32_t* buffer, size_t bufferSize, const char32_t* format, va_list);



int kscanf(const char* format, ...);
int kfscanf(FILE*, const char* format, ...);
int kvscanf(const char* format, va_list);
int kvfscanf(FILE*, const char* format, va_list);

int ksscanf(const char* buffer, const char* format, ...);
int kvsscanf(const char* buffer, const char* format, va_list);
int ksnscanf(const char* buffer, size_t bufferSize, const char* format, ...);
int kvsnscanf(const char* buffer, size_t bufferSize, const char* format, va_list);


int kwscanf(const wchar_t* format, ...);
int kfwscanf(FILE*, const wchar_t* format, ...);
int kvwscanf(const wchar_t* format, va_list);
int kvfwscanf(FILE*, const wchar_t* format, va_list);

int kswscanf(const wchar_t* buffer, const wchar_t* format, ...);
int kvswscanf(const wchar_t* buffer, const wchar_t* format, va_list);
int ksnwscanf(const wchar_t* buffer, size_t bufferSize, const wchar_t* format, ...);
int kvsnwscanf(const wchar_t* buffer, size_t bufferSize, const wchar_t* format, va_list);


int k16scanf(const char16_t* format, ...);
int kf16scanf(FILE*, const char16_t* format, ...);
int kv16scanf(const char16_t* format, va_list);
int kvf16scanf(FILE*, const char16_t* format, va_list);

int ks16scanf(const char16_t* buffer, const char16_t* format, ...);
int kvs16scanf(const char16_t* buffer, const char16_t* format, va_list);
int ksn16scanf(const char16_t* buffer, size_t bufferSize, const char16_t* format, ...);
int kvsn16scanf(const char16_t* buffer, size_t bufferSize, const char16_t* format, va_list);


int k32scanf(const char32_t* format, ...);
int kf32scanf(FILE*, const char32_t* format, ...);
int kv32scanf(const char32_t* format, va_list);
int kvf32scanf(FILE*, const char32_t* format, va_list);

int ks32scanf(const char32_t* buffer, const char32_t* format, ...);
int kvs32scanf(const char32_t* buffer, const char32_t* format, va_list);
int ksn32scanf(const char32_t* buffer, size_t bufferSize, const char32_t* format, ...);
int kvsn32scanf(const char32_t* buffer, size_t bufferSize, const char32_t* format, va_list);


_KSN_END
