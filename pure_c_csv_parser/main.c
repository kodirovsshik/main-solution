
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define errexit(code, fmt, ...) { fprintf(stderr, fmt __VA_OPT__(,) __VA_ARGS__); exit(code);}
 
const char* get_filename(const char* s)
{
	if (!s) return s;

	const char* p = s;
	while (*s != 0)
	{
		if (*s == '\\' || *s == '/')
			p = s + 1;
		++s;
	}
	return p;
}

const char* csv_delim = ";\t\n";
void skip_delim(FILE* f, const char* delims)
{
	int delim = fgetc(f);
	if (delim < 0)
		return;
	if (strchr(delims, delim) != NULL)
	{
		ungetc(delim, f);
		return;
	}

	while (true)
	{
		int c = fgetc(f);
		if (c < 0) return;
		if (c == delim)
			break;
		else
			ungetc(c, f);
	}
}

bool freadable(FILE* f)
{
	return !feof(f) && !ferror(f);
}

typedef char* str;

str read_string(FILE* f, const char* delim)
{
	size_t i = 0, cap = 0;
	str p = NULL;

	while (true)
	{
		int c = fgetc(f);
		if (c < 0)
			break;
		if (strchr(delim, c) != NULL)
			break;

		if (i == cap)
		{
			cap = 2 * cap + 1;
			str p1 = realloc(p, cap + 1);
			if (!p1) goto err_cleanup;
			p = p1;
		}
		p[i++] = (char)c;
	}

	goto success_cleanup;


err_cleanup:
	free(p);
	p = NULL;

success_cleanup:
	if (p) p[i] = 0;
	return p;
}


int main(int argc, str argv[])
{
	if (argc != 2)
		errexit(1, "Error\n\tExample usage: %s <path>\n", get_filename(argv[0]));
}
