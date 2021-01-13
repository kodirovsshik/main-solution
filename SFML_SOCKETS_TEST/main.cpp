#define SFML_STATIC

#include <conio.h>
#include <cstdio>
#include <iostream>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Err.hpp>


#pragma warning(disable : 4996)
#pragma comment(lib, "sfml-network-s.lib")
#pragma comment(lib, "sfml-system-s.lib")

void restart(int exit_code, char* program_name)
{
	system(program_name);
	exit(exit_code);
}

int main(int argc, char** argv)
{



	while (_getch() != '\n');
	return 0;
}