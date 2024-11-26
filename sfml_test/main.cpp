
#include <print>

#include <WinSock2.h>

#include <sfml/Network.hpp>

#pragma comment(lib, "sfml-system-s")
#pragma comment(lib, "sfml-network-s")
#pragma comment(lib, "ws2_32")


#define PORT 9
#define err(...) { std::println(__VA_ARGS__); exit(1); }

void udp_client()
{
	char data[102]{};
	const uint8_t mac[] = { 0xC8, 0x60, 0x00, 0x06, 0x83, 0x2C };

	memset(data, 0xFF, 6);
	for (int i = 1; i <= 16; ++i) memcpy(data + 6 * i, mac, 6);

	sf::IpAddress addr("192.168.100.102");

	sf::UdpSocket s;
	s.send(data, std::size(data), addr, PORT);
	std::println("Send {} bytes to {}:{}", std::size(data), addr.toString(), PORT);
}

void udp_server()
{
	sf::UdpSocket s;
	auto st = s.bind(PORT);
	if (st != s.Done)
		err("Failed to bind to UDP port {}", PORT);

	std::println("Listening on UDP port {}", PORT);

	std::vector<char> buffer(65536);
	
	while (true)
	{
		sf::IpAddress addr;
		uint16_t port;
		size_t sz;
		s.receive(buffer.data(), buffer.size(), sz, addr, port);

		std::println("Recieved {} bytes from {}:{}", sz, addr.toString(), port);
	}
}

int main()
{
	if (0)
		udp_server();
	else
		udp_client();
}
