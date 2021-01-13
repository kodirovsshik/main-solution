#include <conio.h>
#include <stdio.h>

#include <D:/Projects/C++/Solution/upnp_lib/natpmp.h>

#ifdef _DEBUG
#pragma comment(lib, "D:/Projects/C++/Solution/Debug/upnp_lib.lib")
#else
#pragma comment(lib, "D:/Projects/C++/Solution/Release/upnp_lib.lib")
#endif

void redirect(uint16_t privateport, uint16_t publicport, int natpmp_protocol, uint32_t lifetile)
{
	if (natpmp_protocol != NATPMP_PROTOCOL_TCP && natpmp_protocol != NATPMP_PROTOCOL_UDP)
		return;
	int r;
	natpmp_t natpmp;
	natpmpresp_t response;
	initnatpmp(&natpmp, 0, 0);
	sendnewportmappingrequest(&natpmp, natpmp_protocol, privateport, publicport, lifetile);
	do {
		fd_set fds;
		struct timeval timeout;
		FD_ZERO(&fds);
		FD_SET(natpmp.s, &fds);
		getnatpmprequesttimeout(&natpmp, &timeout);
		select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
		r = readnatpmpresponseorretry(&natpmp, &response);
	} while (r == NATPMP_TRYAGAIN);
	printf("mapped public port %hu to localport %hu liftime %u\n",
		response.pnu.newportmapping.mappedpublicport,
		response.pnu.newportmapping.privateport,
		response.pnu.newportmapping.lifetime);
	closenatpmp(&natpmp);
}

int main()
{
	WSAData wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	
	redirect(55501, 4564, NATPMP_PROTOCOL_TCP, 3600);

	int c;
	scanf_s("%i", &c);
	if (c == 1)
	{
		
	}
	
	WSACleanup();
	_getch();
	return 0;
}