#include <cstdio>
#include <conio.h>
#include <winsock2.h>

#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")

int main()
{
	char c;
	long ip;
	short port;

	WSAData data;
	WSAStartup(MAKEWORD(2, 2), &data);

	printf("%s", "1 - Create\n2 - Connect\n");
	do
	{
		printf("%s", "Select: ");
		rewind(stdin);
	} while (scanf("%hhi", &c) != 1);
	
	if (c == 1)
	{
		do
		{
			printf("%s", "Enter port: ");
			rewind(stdin);
		} while (scanf("%hu", &port) != 1);

		int sock = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));

		saddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(port);

		if (bind(sock, (sockaddr*)&saddr, sizeof(saddr)) == -1)
		{
			printf("%s%hu\n", "failed to bind to port ", port);
			while (_getch() != 13);
			return 0;
		}
		if (listen(sock, SOMAXCONN) == -1)
		{
			printf("%s%hu\n", "failed to listen on port ", port);
			while (_getch() != 13);
			return 0;
		}
		printf("Waiting for connections...\n");

		sockaddr_in caddr;
		int len = sizeof(caddr);
		int clientsock = accept(sock, (sockaddr*)&caddr, &len);
		if (clientsock != SOCKET_ERROR)
		{
			printf("%s", "Connected");
		}
		else
			printf("%s", "Errored");
		
		while (_getch() != 13);

		closesocket(sock);
		shutdown(sock, 0);
	}
	else
	{
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		char ipstr[16];

		unsigned long ip = -1;
		do
		{
			printf("%s", "Enter IP: ");
			rewind(stdin);
			scanf("%15s", ipstr);
		}
		while ((ip = inet_addr(ipstr)) == -1);

		do
		{
			printf("%s", "Enter port: ");
			rewind(stdin);
		} while (scanf("%hu", &port) != 1);

		printf("%i\n", ip == INADDR_LOOPBACK);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.S_un.S_addr = ip;

		if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == -1)
			printf("%s", "Failed to connect\n");
		else
			printf("%s", "Connected");
		closesocket(sock);
		shutdown(sock, 0);
		while (_getch() != 13);
		return 0;
	}

	WSACleanup();
	return 0;
}