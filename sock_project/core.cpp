
#include <thread>
#include <list>
#include <mutex>
#include <SFML/Network.hpp>

#include <ksn/ksn.hpp>

#include <stdio.h>

#include <filesystem>

#include "client_descriptor.hpp"
#include "logger.hpp"


//#pragma comment(lib, "sfml-network-s-d.lib")
//#pragma comment(lib, "sfml-system-s-d.lib")
//#pragma comment(lib, "ws2_32.lib")

#pragma warning(disable : 4996)



std::string receive_python_message(sf::TcpSocket& sock)
{
	size_t temp;
	std::string result;
	char data[4097];
	//memset(data, 0, sizeof(data));

	do
	{
		sock.receive((void*)data, 4096, temp);
		data[temp] = 0;
		result += std::string(data);
	} 
	while (temp == 4096);

	sock.send("All fine.", 9);
	result.clear();

	do
	{
		sock.receive((void*)data, 4096, temp);
		data[temp] = 0;
		result += std::string(data);
	} 	while (temp == 4096);

	sock.send("All fine.", 9);


	return result;
}


void server_client_wrapper(sf::TcpSocket& s, std::atomic_bool& done)
{
	void server_client_processor(sf::TcpSocket&);
	server_client_processor(s);
	done = true;
}





int core(int argc, char** argv)
{
	setlocale(LC_ALL, "");
	

	sf::TcpListener server;
	std::list<client_descriptor_t> clients;

	logger_t core_logger("core", { stdout });
	
	char log_folder_name[64];
	{
		time_t start_time = time(nullptr);
		struct tm start_time_data = *localtime(&start_time);
		strftime(log_folder_name, 64, "%F_%H.%M.%S", &start_time_data);
		std::filesystem::create_directory(log_folder_name);
	}
	auto open_file = [&]
	(const char* name, const char* mode = "w+b") -> FILE*
	{
		char name_buffer[_MAX_PATH];
		snprintf(name_buffer, _MAX_PATH, "%s/%s", log_folder_name, name);
		return fopen(name_buffer, mode);
	};



	server.listen(9090);

	while (1)
	{
		clients.emplace_front();
		if ((err_status = clients.front().accept(server)) != client_descriptor_t::error_t::E_OK)
		{
			core_logger.error("Error: Failed to accept incoming connection, status %i", err_status);
		}

		for (auto p = clients.begin(); p != clients.end(); ++p)
		{
			if (p->done)
			{
				clients.erase(p);
			}
		}
	}
}
