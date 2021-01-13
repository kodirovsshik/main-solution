#include <SFML/Network.hpp>
#include <stdio.h>
#include <stdarg.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "sfml-system-s-d.lib")
#pragma comment(lib, "sfml-network-s-d.lib")

[[noreturn]] void err(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	vfprintf(stderr, format, ap);

	va_end(ap);

	fprintf(stderr, "\nWSA error code: %i\n", WSAGetLastError());
	exit(1);
}

int main()
{
	int dbg;

	if ((dbg = WSAStartup(MAKEWORD(2, 2), (struct WSAData*)malloc(sizeof(struct WSAData)))) != 0)
		return -1;

	int sockL = -1;
	int sockC = -1;
	sockL = socket(PF_INET, SOCK_STREAM, 0);

	sockaddr_in addrL;
	//memset(&addrL, 0, sizeof(addrL));
	addrL.sin_addr.s_addr = 0;
	addrL.sin_port = htons(27014);
	addrL.sin_family = AF_INET;

	dbg = bind(sockL, (struct sockaddr*) & addrL, sizeof(addrL));
	dbg = listen(sockL, SOMAXCONN);

	sockaddr_in addrC;
	int addrClen = sizeof(addrC);
	sockC = accept(sockL, (struct sockaddr*)&addrC, &addrClen);

}