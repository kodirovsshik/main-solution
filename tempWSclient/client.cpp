#include <SFML/Network.hpp>
#include <stdio.h>
#include <stdarg.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "sfml-system-s-d.lib")
#pragma comment(lib, "sfml-network-s-d.lib")

#pragma warning(disable : 4996)
//
//[[noreturn]] void err(const char* format, ...)
//{
//	va_list ap;
//	va_start(ap, format);
//
//	vfprintf(stderr, format, ap);
//
//	va_end(ap);
//
//	fprintf(stderr, "\nWSA error code: %i\n", WSAGetLastError());
//	exit(1);
//}
//
//void loop()
//{
//	SOCKET sock = socket(PF_INET, SOCK_STREAM, 0);
//	sockaddr_in saddr;
//	saddr.sin_addr.s_addr = inet_addr("25.91.213.205");
//	saddr.sin_port = htons(27014);
//	saddr.sin_family = AF_INET;
//
//	while (1)
//	{
//		if (-1 != connect(sock, (sockaddr*)& saddr, sizeof(saddr)))
//			break;
//		printf("Connection failed, retrying\n");
//		Sleep(1000);
//	}
//	printf("\vSuccess!\n\v");
//	closesocket(sock);
//}

int main()
{
	FILE* f = fopen("a.txt", "w");
	for (int i = 0; i < 1000; ++i)
	{
		char buff[8];
		sprintf(buff, "%i", i);
		char* p = buff;
		while (*p)
			fprintf(f, "%c\n", *p);

	}
	return 0;

	//while (1)
	//	loop();
}