
#include <print>
#include <span>
#include <ranges>
#include <thread>
#include <semaphore>
#include <stacktrace>
#include <regex>
#include <unordered_map>

#define SFML_STATIC
#include <SFML/Network.hpp>

#pragma comment(lib, "sfml-network-s")
#pragma comment(lib, "sfml-system-s")
#pragma comment(lib, "ws2_32")

constexpr int server_port = 9001;
constexpr int accept_delay_ms = 10;

#define stringize1(x) #x
#define stringize(x) stringize1(x)
#define assert_or_panic(cond, fmt, ...) [&]{ if (cond) return; std::print(stderr, __FILE__ ":" stringize(__LINE__) ":\nASSERTION FAILED: " #cond "\n"); std::print(stderr, fmt __VA_OPT__(,) __VA_ARGS__); exit(-1); }()
#define assert_or_throw(cond, fmt, ...) [&]{ if (cond) return; throw std::format(fmt __VA_OPT__(,) __VA_ARGS__); }()



struct http_header_entry
{
	std::string property;
	std::string value;
};
using http_message_headers = std::vector<http_header_entry>;
using http_message_payload = std::vector<std::byte>;

struct http_message_data
{
	http_message_headers headers;
	http_message_payload payload;

	template<class CharT, size_t N>
	void append_payload(const CharT (&str)[N])
	{
		append_payload(str, sizeof(str) - 1);
	}

	void append_payload(const void* p, size_t n)
	{
		memcpy(get_more_payload_storage(n), p, n);
	}

private:
	void* get_more_payload_storage(size_t additional_payload_size)
	{
		const size_t payload_offset = payload.size();
		payload.resize(payload.size() + additional_payload_size);
		return payload.data() + payload_offset;
	}
};

struct http_request
{
	std::string method;
	std::string path;
	std::string http_version;
	http_message_data data;
};
struct http_response
{
	//std::string http_version = "HTTP/1.1";
	std::string status;
	http_message_data data;
};





template<class UpstreamSocket>
class socket_wrapper
{
	UpstreamSocket socket;
	std::optional<char> c;


public:
	socket_wrapper(UpstreamSocket& socket) : socket(socket) {}

	bool has_ready_data() const noexcept
	{
		return c.has_value();
	}
	char peek() const
	{
		return c.value();
	}
	bool advance_if_has_no_data()
	{
		if (!has_ready_data())
			return advance();
		return true;
	}
	void discard() noexcept
	{
		this->c = std::nullopt;
	}
	bool advance(size_t n = 1)
	{
		char buff;
		size_t recv = 0;

		while (n --> 0)
		{
			(void)socket.receive(&buff, 1, recv);
			if (!recv) break;
		}

		if (recv)
			c = buff;
		else
			this->discard();

		return recv;
	}
};



template<class Socket>
bool http_skip(socket_wrapper<Socket>& socket, const std::string_view word_)
{
	std::span<const char> word(word_.data(), word_.size());
	while (true)
	{
		if (word.size() == 0)
			return true;

		socket.advance_if_has_no_data();
		if (!socket.has_ready_data() || socket.peek() != word.front())
			return false;

		word = word.subspan(1);
		socket.discard();
	}
}

template<class Socket>
std::string http_read(socket_wrapper<Socket>& socket, char delim, const std::string_view footer = "")
{
	static constexpr size_t max_word_size = 128;

	std::string result;

	socket.advance_if_has_no_data();
	while (true)
	{
		if (!socket.has_ready_data() || socket.peek() == delim)
			break;
		result += socket.peek();
		socket.advance();
	}

	if (socket.has_ready_data() && socket.peek() == delim)
		socket.discard();

	if (!http_skip(socket, footer))
		result.clear();

	return result;
}

template<class Socket>
std::string http_read_word(socket_wrapper<Socket>& socket)
{
	return http_read(socket, ' ');
}
template<class Socket>
std::string http_read_property_name(socket_wrapper<Socket>& socket)
{
	return http_read(socket, ':', " ");
}
template<class Socket>
std::string http_readln(socket_wrapper<Socket>& socket)
{
	return http_read(socket, '\r', "\n");
}



#define assert_or_bad_request(cond) { if (!(cond)) { result = {}; return result;} }

template<class Socket>
size_t socket_read_buff_exact(void* buff, size_t bytes_to_read, Socket& socket)
{
	const size_t total_bytes = bytes_to_read;

	while (bytes_to_read)
	{
		size_t received = 0;
		(void)socket.receive(buff, bytes_to_read, received);

		if (received == 0)
			break;

		bytes_to_read -= received;
		buff = (uint8_t*)buff + received;
	}

	return total_bytes - bytes_to_read;
}

template<class Socket>
http_request receive_http_request(Socket& raw_socket)
{
	socket_wrapper<Socket&> socket(raw_socket);
	http_request result;

	result.method = http_read_word(socket);
	result.path = http_read_word(socket);
	assert_or_bad_request(http_skip(socket, "HTTP/"));
	result.http_version = http_readln(socket);

	size_t payload_expected_size = 0;
	static constexpr size_t payload_chunked = -1;

	std::string property, value;
	while (true)
	{
		assert_or_bad_request(socket.advance_if_has_no_data());
		if (socket.peek() == '\r')
		{
			assert_or_bad_request(http_skip(socket, "\r\n"));
			break;
		}

		const auto& new_entry = result.data.headers.emplace_back(http_header_entry{ http_read_property_name(socket), http_readln(socket) });
		assert_or_bad_request(!new_entry.property.empty() && !new_entry.value.empty());

		if (new_entry.property == "Transfer-Encoding" && new_entry.value == "chunked")
			payload_expected_size = -1;

		if (new_entry.property == "Content-Length" && payload_expected_size != payload_chunked)
			assert_or_bad_request(std::from_chars(&new_entry.value.front(), &new_entry.value.back(), payload_expected_size).ec == std::errc{});
	}
	
	if (payload_expected_size == 0)
		return result;

	auto& payload = result.data.payload;
	
	if (payload_expected_size == payload_chunked)
	{
		//TODO: test
		while (true)
		{
			size_t chunk_size = 0;

			const auto chunk_size_str = http_readln(socket);
			if (std::from_chars(&chunk_size_str.front(), &chunk_size_str.back(), chunk_size).ec != std::errc{})
				break;
			
			const size_t buffer_offset = payload.size();
			payload.resize(payload.size() + chunk_size);

			const size_t received = socket_read_buff_exact(payload.data() + buffer_offset, chunk_size, raw_socket);
			if (received != chunk_size)
				break;

			assert_or_bad_request(http_skip(socket, "\r\n"));

			if (chunk_size == 0)
				break;
		}
	}
	else
	{
		size_t received = 0;
		payload.resize(payload_expected_size);
		received = socket_read_buff_exact(payload.data(), payload_expected_size, raw_socket);
		assert_or_bad_request(received == payload_expected_size);
	}

	return result;
}

void print_http_request_first_line(const http_request& request)
{
	std::print("{} {} HTTP/{}\n", request.method, request.path, request.http_version);
}
void print_http_msg_headers(const http_message_headers& headers)
{
	for (const auto& [property, value] : headers)
		std::print("{}: {}\n", property, value);
}
void print_http_msg_payload_info(const http_message_payload& payload)
{
	if (payload.size())
		std::print("<payload of size {} bytes>\n\n", payload.size());
	else
		std::print("<no payload>\n\n");
}
void print_http_message_data(const http_message_data& message)
{
	print_http_msg_headers(message.headers);
	print_http_msg_payload_info(message.payload);
}
void print_http_request(const http_request& request)
{
	print_http_request_first_line(request);
	print_http_message_data(request.data);
}

struct thread_data
{
	sf::TcpSocket client;
	std::string log;
	size_t request_id;
};

struct logger
{
	std::string log;

	template<class... T>
	void operator()(const std::string_view& fmt, const T& ...args)
	{
		log.append(std::vformat(fmt, std::make_format_args(args...)));
	}

	~logger()
	{
		std::print("{}\n", log);
	}
};



template<class T> concept span_like = requires(T t) { t.data(); t.size(); };

template<span_like T>
void send(sf::TcpSocket& client, const T& s)
{
	(void)client.send(s.data(), s.size() * sizeof(*s.data()));
}
template<size_t N>
void send(sf::TcpSocket& client, const char (&str)[N])
{
	send(client, std::string_view(str, N - 1));
}

void send_http_response(const http_response& response, sf::TcpSocket& client)
{
	send(client, "HTTP/1.1 ");
	send(client, response.status);

	for (const auto& [property, value] : response.data.headers)
	{
		send(client, "\r\n");
		send(client, property);
		send(client, ": ");
		send(client, value);
	}

	send(client, "\r\n\r\n");
	send(client, response.data.payload);
}

struct request_processing_context
{
	http_request request;
	http_response response;
	logger log;
};



const std::unordered_map<int, const std::string_view> http_codes_names = {
	{200, "OK"},
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{500, "Internal Server Error"},
};

template<class K, class V>
std::optional<V> maybe_lookup(const std::unordered_map<K, V>& cont, const K& code)
{
	const auto iter = http_codes_names.find(code);
	if (iter == cont.end())
		return std::nullopt;

	return iter->second;
}

void response_fill_code(http_response& response, int code)
{
	const std::string_view explanation = maybe_lookup(http_codes_names, code).value_or("");
	response.status = std::format("{}{}{}", code, explanation.size() > 0 ? " " : "", explanation);
}

void handle_request_by_simple_response(request_processing_context& context, int code)
{
	response_fill_code(context.response, code);

	/*
	
	sequence{
		//void_element{"!DOCTYPE", {"html"}, {}},
		element{"html", {}, {{"lang", "en"}}, element{}},
		element{"body", {}, {}, sequence{
			element{"h1", {}, {{"align", "center"}}, code},
			void_element{"hr", {}, {}}
			element{"h3", {}, {{"align", "center"}}, explanation},
		}}
	}

	html(lang=en){
		head{}
		body{
			h1(align=center){code},
			hr,
			h3(align=center){explanation}
		}
	}
	
	*/
}

void handle_request_for_main_page(request_processing_context& context)
{
	response_fill_code(context.response, 200);
	context.response.data.headers.push_back({ "Content-Language", "en" });
	context.response.data.headers.push_back({ "Content-Type", "text/plain; Charset=UTF-8" });
	context.response.data.append_payload(u8"This web page was brought to you by a test C++™ HTTP/1.1 web server\nShout out to Bjarne Stroustroup");
}

void handle_request_error_fallback(request_processing_context& context)
{
	context.response = {};
	response_fill_code(context.response, 500);
}

void client_main(thread_data& data)
{
	request_processing_context context;

	auto& [request, response, log] = context;
	auto& client = data.client;

	std::string_view s = "";

	log("{}:{} - ", client.getRemoteAddress().value().toString(), client.getRemotePort());

	request = receive_http_request(client);

	try
	{
		if (request.method != "GET")
			handle_request_by_simple_response(context, 400);
		else if (std::regex_match(request.path, std::regex(R"(\/(submit)?(\?[\w=&]*)?)")))
			handle_request_for_main_page(context);
		else
			handle_request_by_simple_response(context, 404);
	}
	catch (...)
	{
		handle_request_error_fallback(context);
	}

	//response.data.headers.push_back({ "Connection", "close" });
	send_http_response(response, client);

	client.disconnect();

	log("{}", response.status);
}



template<size_t N>
class semaphore_lock_guard
{
	std::counting_semaphore<N>& semaphore;

public:
	semaphore_lock_guard(std::counting_semaphore<N>& semaphore) : semaphore(semaphore) { semaphore.acquire(); }
	~semaphore_lock_guard() { semaphore.release(); }
};

struct alignas(std::hardware_destructive_interference_size) thread_local_context
{
	std::thread thread;
	thread_data data;
};

//void maybe_join(std::thread& t)
//{
//	if (t.joinable()) t.join();
//}

struct thread_pool
{
	std::vector<thread_local_context> threads;
	std::vector<size_t> free_threads;
	std::binary_semaphore semaphore{ 1 };

	size_t allocate_thread()
	{
		semaphore_lock_guard lock(semaphore);

		if (free_threads.size()) [[likely]]
		{
			const size_t id = free_threads.back();
			free_threads.pop_back();
			//maybe_join(threads[id].thread); //TODO: figure if it's needed
			threads[id].thread.join();
			return id;
		}

		const size_t id = threads.size();
		threads.emplace_back();
		return id;
	}

	void release_thread(size_t pool_id)
	{
		threads[pool_id].data = {};
		semaphore_lock_guard lock(semaphore);
		free_threads.push_back(pool_id);
	}
};



std::string get_filename(const std::string& s)
{
	return s.substr(s.rfind('\\') + 1);
}

void print_stacktrace()
{
	const auto stacktrace = std::stacktrace::current(1);

	std::print("Stacktrace:\n");
	for (const auto& entry : stacktrace)
	{
		std::print("{:60} {}:{}\n", entry.description(), get_filename(entry.source_file()), entry.source_line());
	}
}

void client_worker_wrapper(thread_pool& pool, size_t pool_id)
{
	try
	{
		client_main(pool.threads[pool_id].data);
	}
	catch (...)
	{
		std::print("Worker {} has crashed with unknown exception\n", pool_id);
		print_stacktrace();
	}
	
	pool.release_thread(pool_id);
}

void server_main()
{
	sf::TcpListener server;
	assert_or_panic(server.listen(server_port) == sf::Socket::Status::Done, "Failed to listen on {}", server_port);
	std::print("Listening on http://localhost:{}\n", server_port);

	server.setBlocking(false);

	thread_pool pool;
	size_t request_id = 0;

	while (true)
	{
		sf::TcpSocket client;
		const auto status = server.accept(client);

		if (status != sf::Socket::Status::Done)
		{
			assert_or_panic(status == sf::Socket::Status::NotReady, "Listener socket died");
			std::this_thread::sleep_for(std::chrono::milliseconds(accept_delay_ms));
			continue;
		}

		const size_t pool_id = pool.allocate_thread();
		auto& thread_local_context = pool.threads[pool_id];

		thread_local_context.data.request_id = ++request_id;
		thread_local_context.data.client = std::move(client);
		thread_local_context.thread = std::thread(client_worker_wrapper, std::ref(pool), pool_id);

		continue;
	}
}

int main()
{
	try
	{
		server_main();
	}
	catch (const std::string& s)
	{
		std::print("Exception std::string\nwhat() = {}\n", s);
		print_stacktrace();
	}
	catch (const std::exception& e)
	{
		std::print("Exception {}\nwhat() = {}\n", typeid(e).name(), e.what());
		print_stacktrace();
	}
	catch (...)
	{
		std::print("Exception <unknown>\n");
		print_stacktrace();
	}
}
