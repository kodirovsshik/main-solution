
#ifndef _CLIENT_DESCRIPTOR_HPP_
#define _CLIENT_DESCRIPTOR_HPP_

#include <thread>
#include <atomic>

#include <SFML/Network.hpp>



class client_descriptor_t
{
	std::thread* pthread;
	sf::TcpSocket* psocket;

public:
	std::atomic_bool done;
	
	enum class error_t : int
	{
		E_OK = 0,
		E_INVALID_PARAM = 1,
		E_OUT_OF_MEMORY = 2,
	}

	client_descriptor_t() noexcept : pthread(nullptr), psocket(nullptr) {}
	client_descriptor_t(const client_descriptor_t&) = delete;
	client_descriptor_t(client_descriptor_t&& other) noexcept
		: psocket(other.psocket), pthread(other.pthread)
	{
		other.psocket = nullptr;
		other.pthread = nullptr;
	}

	client_descriptor_t& operator=(const client_descriptor_t&) = delete;
	client_descriptor_t& operator=(client_descriptor_t&& other) = delete;

	int accept(sf::TcpListener& server)
	{
		void server_client_wrapper(sf::TcpSocket&, std::atomic_bool&);

		if (pthread || psocket) return E_INVALID_PARAM;

		psocket = new (std::nothrow) sf::TcpSocket;
		if (psocket == nullptr) return E_OUT_OF_MEMORY;

		pthread = new (std::nothrow) std::thread();
		if (pthread == nullptr) return E_OUT_OF_MEMORY;

		server.accept(*this->psocket);
		*this->pthread = std::thread(server_client_wrapper, std::ref(*this->psocket), std::ref(this->done));

		return E_OK;
	}

	~client_descriptor_t() noexcept
	{
		delete this->psocket;
		delete this->pthread;
	}
};



#endif //!_CLIENT_DESCRIPTOR_HPP_
