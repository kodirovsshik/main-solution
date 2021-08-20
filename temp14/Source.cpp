
#define SFML_STATIC
#include <SFML/Network.hpp>

#include <ksn/stuff.hpp>


#pragma warning(disable : 26812 4996)
#pragma comment(lib, "sfml-main-d")
#pragma comment(lib, "sfml-system-s-d")
#pragma comment(lib, "sfml-network-s-d")

#pragma comment(lib, "ws2_32")

#pragma comment(lib, "libksn_stuff")

#define assert_err(code) if (auto __code = code; __code != 0) {return __code;} else []{}()


size_t received = -1;

int main1()
{
	const size_t alloc_size = 1024 * 1024 * 1024;

	uint8_t (&buffer)[alloc_size] = *(uint8_t(*)[alloc_size])malloc(alloc_size);
	memset(buffer, 0x10, sizeof(buffer));

	sf::TcpSocket sock;
	assert_err(sock.connect("192.168.100.1", 53));
	sock.setBlocking(true);
	while (1)
		assert_err(sock.send(buffer, alloc_size));
	//assert_err(sock.receive(buffer, alloc_size, received));
	
	//ksn::memory_dump(buffer, received, 01, ksn::memory_dump.no_space, fopen("data.txt", "wb"));

	return 0;
}

int main()
{
	int x = main1();
	printf("%zi\n", received);
	return x;
}
