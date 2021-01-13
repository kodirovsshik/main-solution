#include <SFML/Network.hpp>
#include <stdio.h>
#include <stdarg.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "sfml-system-s-d.lib")
#pragma comment(lib, "sfml-network-s-d.lib")

#pragma warning(disable : 4996)

[[noreturn]] void err(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	vfprintf(stderr, format, ap);

	va_end(ap);

	fprintf(stderr, "\nWSA error code: %i\n", WSAGetLastError());
	exit(1);
}

void loop()
{
	SOCKET sock = socket(PF_INET, SOCK_STREAM, 0);
	sockaddr_in saddr;
	saddr.sin_addr.s_addr = inet_addr("25.91.213.205");
	saddr.sin_port = htons(27014);
	saddr.sin_family = AF_INET;

	while (1)
	{
		if (-1 != connect(sock, (sockaddr*)& saddr, sizeof(saddr)))
			break;
		printf("Connection failed, retrying\n");
		Sleep(1000);
	}
	printf("\vSuccess!\n\v");
	closesocket(sock);
}

int main()
{
	if (WSAStartup(MAKEWORD(2, 2), (struct WSAData*)malloc(sizeof(struct WSAData))) != 0)
		return -1;

	while (1)
		loop();
}