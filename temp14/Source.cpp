
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <sstream>

#pragma warning(disable : 4996)

int main()
{

	int x = 42;

	char s1[64] = { 0 };
	sprintf(s1, "%i", x);

	std::string s2 = std::to_string(x);

	std::stringstream ss;
	ss << x;
	std::string s3 = ss.str();

	char s4[64];
	itoa(x, s4, 10);



	return 0;
}
