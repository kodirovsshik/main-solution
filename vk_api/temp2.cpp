// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdio.h>
#include <time.h>
#include <conio.h>

#include <thread>
#include <vector>
#include <mutex>

#include <curl/curl.h>

#include <ksn/ksn.hpp>

#include <json.hpp>

#pragma warning(disable : 4996 4839 6031 26812 )

#define LAST_API "5.126"

uint64_t rand64()
{
	uint64_t result = uint64_t(rand());
	result *= rand(); //-V760
	result *= result;
	result *= rand();
	result *= result;
	result *= rand();
	return result;
}

_KSN_BEGIN

int curl_writer(char* data, size_t size, size_t length, std::string* pstring)
{
	if (!pstring)
		return 0;
	pstring->append(data, length * size);
	return int(length * size);
};

std::string http_request_perform(CURL* curl, const std::string& url, const std::string& data)
{
	//printf("%s?%s\n", url.data(), data.data());
	if (!curl)
		return "";

	std::string response;
	char error_buffer[CURL_ERROR_SIZE] = { 0 };

	curl_easy_setopt(curl, CURLOPT_URL, url.data()); //-V111
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer); //-V111
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response); //-V111
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.data()); //-V111
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "");
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer); //-V111

	auto request_result = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	if (1)
	{
		static int counter = 0;
		char buffer[64];

		FILE* fd = nullptr;
		while (fd == nullptr)
		{
			sprintf(buffer, "response%i_%llu.html", counter, rand64());
			fd = fopen(buffer, "w");
		}

		fwrite(
			request_result == CURLE_OK ? response.data() : error_buffer,
			sizeof(char),
			request_result == CURLE_OK ? response.length() : strlen(error_buffer),
			fd
		);

		fclose(fd);

		counter++;

		if (1)
		{
			system(buffer);
		}
	}

	if (request_result == CURLE_OK)
		return response;
	else
		return error_buffer;
}

std::string http_request(const std::string& url, const std::unordered_map<std::string, std::string>& params)
{
	CURL* curl = curl_easy_init();
	if (!curl)
		return "";

	std::string data;

	auto url_preprocess = [&](const std::string& string) -> std::string
	{
		std::string result;
		for (const char& c : string)
		{
			if ((48 <= c && c <= 57) ||
				(65 <= c && c <= 90) ||
				(97 <= c && c <= 122) ||
				(c == '~' || c == '!' || c == '*' || c == '(' || c == ')' || c == '\'')
				)
			{
				result.push_back(c);
			}
			else
			{
				result.push_back('%');
				char buff[4];
				sprintf(buff, "%.2hhX", c);
				result.append(buff);
			}
		}
		return result;
	};

	for (const auto& pair : params)
	{
		data += pair.first;
		data.push_back('=');
		data += url_preprocess(pair.second);
		data.push_back('&');
	}

	return http_request_perform(curl, url, data);
}

class VKApi
{
public:
	enum class Device
	{
		PC,
		Android,
		IPhone,
		IPad,
		WP,
		KateMobile,
		Messenger,
		Snapster,
		Nokia,
		Lynt,
		Vika,

		count
	};


	std::string access_token;
	std::string self_id;
	std::string captcha_sid;


	struct auth_result
	{
		nlohmann::json response;
		std::string captcha_url;
		bool ok = false;
		bool need_capcha = false;
		bool need_2fa_code = false;
		bool invalid_user = false;
		bool parse_error = false;
		bool need_2fa_sms = false;
	};



private:
	static std::string client_id[(int)Device::count];
	static std::string client_secret[(int)Device::count];


	auth_result post_auth(const std::string& str)
	{
		nlohmann::json parsed_response;
		try
		{
			parsed_response = nlohmann::json::parse(str);
		}
		catch (...)
		{
			auth_result x;
			x.response = str;
			x.parse_error = true;
			return x;
		}

		auth_result result{ parsed_response };
		this->access_token.clear();

		if (parsed_response.find("access_token") != parsed_response.end())
		{
			this->access_token = parsed_response["access_token"].get<std::string>();
			this->self_id = std::to_string(parsed_response["user_id"].get<int>());
			result.ok = true;
			return result;
		}

		if (parsed_response.find("error") != parsed_response.end())
		{
			if (parsed_response["error_type"] == "username_or_password_is_incorrect")
			{
				result.invalid_user = true;
			}
			else if (parsed_response["error"] == "need_validation")
			{
				if (parsed_response["validation_type"] == "2fa_app")
					result.need_2fa_code = true;
				else if (parsed_response["validation_type"] == "2fa_sms")
				{
					result.need_2fa_sms = true;
					this->captcha_sid = parsed_response["validation_sid"].get<std::string>();
				}
			}
			else if (parsed_response["error"] == "need_captcha")
			{
				result.need_capcha = true;
				this->captcha_sid = parsed_response["captcha_sid"].get<std::string>();
				result.captcha_url = parsed_response["captcha_img"].get<std::string>();
			}
		}

		return result;
	}



public:

	auth_result auth(const std::string& username, const std::string& password, Device device = Device::Android)
	{
		return this->post_auth(http_request("https://oauth.vk.com/token",
			{
				{"grant_type", "password"},
				{"username", username},
				{"password", password},
				{"client_id", VKApi::client_id[(int)device]},
				{"client_secret", VKApi::client_secret[(int)device]},
				{"scope", "friends,photos,audio,video,stories,pages,status,notes,messages,wall,offline,docs,groups,notifications,stats"},
				{"2fa_supported", "1"},
				{"lang", "en"},
			}
			));
	}

	auth_result auth_captcha(const std::string& username, const std::string& password, const std::string& captcha, Device device = Device::Android)
	{
		if (this->captcha_sid.empty())
			return this->auth(username, password, device);
		return this->post_auth(http_request("https://oauth.vk.com/token",
			{
				{"grant_type", "password"},
				{"username", username},
				{"password", password},
				{"client_id", VKApi::client_id[(int)device]},
				{"client_secret", VKApi::client_secret[(int)device]},
				{"scope", "friends,photos,audio,video,stories,pages,status,notes,messages,wall,offline,docs,groups,notifications,stats"},
				{"2fa_supported", "1"},
				{"lang", "en"},
				{"captcha_sid", this->captcha_sid},
				{"captcha_key", captcha},
			}
		));
	}

	auth_result auth_2fa(const std::string& username, const std::string& password, const std::string& code, Device device = Device::Android)
	{
		return this->post_auth(http_request("https://oauth.vk.com/token",
				{
					{"grant_type", "password"},
					{"username", username},
					{"password", password},
					{"client_id", VKApi::client_id[(int)device]},
					{"client_secret", VKApi::client_secret[(int)device]},
					{"scope", "friends,photos,audio,video,stories,pages,status,notes,messages,wall,offline,docs,groups,notifications,stats"},
					{"2fa_supported", "1"},
					{"lang", "en"},
					{"code", code}
				}
				));
	}

	auth_result auth_access_token(const std::string& token)
	{
		auth_result x;
		
		x.ok = true;
		this->access_token = token;
		
		try
		{
			auto response = nlohmann::json::parse(this->call("users.get", { }));
			
			if (response.find("error") != response.end())
			{
				x.ok = false;
				if (response["error"]["error_code"].get<int>() == 5)
				{
					x.invalid_user = true;
				}
				return x;
			}

			this->self_id = std::to_string(response["response"][0]["id"].get<unsigned int>());
			return x;
		}
		catch (...)
		{
			this->access_token.clear();
			x.parse_error = true;
		}

		x.ok = false;
		return x;
	}

	std::string call_nowait(const std::string& method, std::unordered_map<std::string, std::string> params = {}, const std::string& version = LAST_API)
	{
		std::string url = "https://api.vk.com/method/" + method;
		params["access_token"] = this->access_token;
		params["v"] = version;
		params["lang"] = "en";
		return http_request(url, params);
	}

	std::string call(const std::string& method, std::unordered_map<std::string, std::string> params = {}, const std::string& version = LAST_API)
	{
		std::string url = "https://api.vk.com/method/" + method;

		params["v"] = version;
		if (params.count("lang") == 0)
			params["lang"] = "en";

		if (this->access_token.length() != 0)
			params["access_token"] = this->access_token;
		else
			params.erase("access_token");

		while (1)
		{
			std::string response = http_request(url, params);
			nlohmann::json parsed_response = nlohmann::json::parse(response);

			if (parsed_response.find("error") != parsed_response.end())
			{
				try
				{
					if (parsed_response["error"]["error_code"].get<int>() == 6)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(1001));
						continue;
					}
				}
				catch (...)
				{
					return response;
				}
			}
			return response;
		}
	}

};

std::string VKApi::client_id[(int)VKApi::Device::count] = {
	"3697615", //PC
	"2274003", //Android
	"3140623", //Iphone
	"3682744", //Ipad
	"3502557", //WP
	"2685278", //Kate Mobile
	"5027722", //VK Messenger
	"4580399", //Snapster (Android)
	"2037484", //Nokia (Symbian)
	"3469984", //Lynt
	"3032107", //Vika (Blackberry)
};
std::string VKApi::client_secret[(int)VKApi::Device::count] = {
	"AlVXZFMUqyrnABp8ncuU", //PC
	"hHbZxrka2uZ6jB1inYsH", //Android
	"VeWdmVclDCtn6ihuP1nt", //Iphone
	"mY6CDUswIVdJLCD3j15n", //Ipad
	"PEObAuQi6KloPM4T30DV", //WP
	"lxhD8OD7dMsqtXIm5IUY", //Kate Mobile
	"Skg1Tn1r2qEbbZIAJMx3", //VK Messenger
	"wYavpq94flrP3ERHO4qQ", //Snapster (Android)
	"gpfDXet2gdGTsvOs7MbL", //Nokia (Symbian)
	"kc8eckM3jrRj8mHWl9zQ", //Lynt
	"NOmHf1JNKONiIG5zPJUu", //Vika (Blackberry)
};

_KSN_END



void read_password(std::string& pass, bool put_endl = true)
{
	pass.clear();
	while (1)
	{
		char c = _getch();

		if (c == 13 || c == 10)
		{
			if (put_endl)
				fwrite("\n", 1, 1, stdout);
			return;
		}

		if (c == '\b')
		{
			if (pass.length())
				pass.pop_back();
		}
		else
			pass.push_back(c);
	}
}



void url_open(const char* url)
{
	char buffer[64];
	FILE* fd;

	do
	{
		sprintf(buffer, "%llu.url", rand64());
		fd = fopen(buffer, "w");
	} while (fd == 0);

	fwrite("[InternetShortcut]\nURL=", sizeof(char), 23, fd);
	fwrite(url, 1, strlen(url), fd);
	fclose(fd);

	system(buffer);
	remove(buffer);
}



void debug_store(const char* msg)
{
	char buffer[64];
	static int counter = 0;
	FILE* fd;

	do
	{
		sprintf(buffer, "debug%i_%llu.txt", counter++, rand64());
		fd = fopen(buffer, "w");
	} while (fd == 0);

	fwrite(msg, 1, strlen(msg), fd);
	fclose(fd);

	if (0)
		system((std::string("start ")  + buffer).data());
}



std::string get_posts(ksn::VKApi& vk, std::vector<int>& posts_ids, const std::string& id)
{
	auto get_posts_helper = [&]
	(size_t from) -> std::string
	{
		std::stringstream ss;
		ss << R"(
var count = 0;
var offset = )";

		ss << 100 + from;

		ss << R"(;
var done = 1;
var first = API.wall.get({ "count": 100 , "owner_id": )";
		ss << id;

		ss << R"(});
count = first.count - 100;
var posts = first.items@.id;

if (count > 2400)
{
count = 2400;
done = 0;
}

while (count > 0)
{
posts = posts + API.wall.get({ "count": 100, "offset" : offset }).items@.id;
count = count - 100;
offset = offset + 100;
};

return { "all": done, "items" : posts }; )";

		return vk.call("execute", { {"code", ss.str().data()} });
	};

	size_t offset = 0;
	bool done;

	posts_ids.clear();

	std::string response;
	do
	{
		response = get_posts_helper(offset);
		nlohmann::json parsed_response;
		try
		{
			parsed_response = nlohmann::json::parse(response);
		}
		catch (...)
		{
			return response;
		}

		if (parsed_response.find("response") == parsed_response.end())
			return response;

		auto code_response = parsed_response["response"];

		if (code_response.find("all") == code_response.end())
			return response;

		done = code_response["all"].get<int>();
		auto more_ids = code_response["items"].get<std::vector<int>>();
		posts_ids.insert(posts_ids.end(), more_ids.begin(), more_ids.end());
	} while (!done);

	return "{}";
}

template <class CharT, class TraitsT = std::char_traits<CharT>, class AllocatorT = std::allocator<CharT>>
class basic_string_autoclean : public
	std::basic_string<CharT, TraitsT, AllocatorT>
{
public:
	template<class... Args>
	basic_string_autoclean(Args&& ...args) :
		std::basic_string<CharT, TraitsT, AllocatorT>(args...)
	{
	}

	~basic_string_autoclean()
	{
		RtlSecureZeroMemory(this->data(), this->capacity() * sizeof(CharT));
	}
};

using string_autoclean = basic_string_autoclean<char>;
using wstring_autoclean = basic_string_autoclean<wchar_t>;

#pragma warning(disable : 26444)

std::string read_token(const std::string& filename = "token.txt")
{
	FILE* fd = fopen(filename.data(), "r");

	if (fd == nullptr)
	{
		return "";
	}

	char buffer[1024];
	fgets(buffer, 1023, fd);
	fclose(fd);

	return buffer;
}

int get_curr_min()
{
	time_t time_since_epoch;
	time(&time_since_epoch);
	return localtime(&time_since_epoch)->tm_min;
}

std::string get_curr_time()
{
	char buff[8];

	time_t time_since_epoch;
	time(&time_since_epoch);

	strftime(buff, 8, "%H:%M", localtime(&time_since_epoch));

	return buff;
}



std::unordered_map<size_t, std::string> name_by_id;
std::vector<size_t> checker_ids;
std::mutex checker_ids_mutex;

[[noreturn]] void checker()
{
	auto clock_f = &std::chrono::high_resolution_clock::now;
	auto t1 = clock_f(), t2 = t1;

	while (1) //-V776
	{
		size_t i = 0;
		size_t lim = 1000;

		std::stringstream ss;

		checker_ids_mutex.lock();
		size_t total = checker_ids.size();
		checker_ids_mutex.unlock();

		while (i < total)
		{
			if (lim > total)
			{
				lim = total;
			}

			size_t current = i - lim;

			checker_ids_mutex.lock();
			for (; i < lim; ++i)
			{
				ss << checker_ids[i] << ',';
			}
			checker_ids_mutex.unlock();

			lim += 1000;
			ss.clear();
		}
	}
}

int main2()
{
	using ksn::VKApi;
	srand((uint32_t)time(0));

	VKApi vk;
	//std::vector<int> posts_ids;
	size_t wall_offset = 0;
	std::string response;
	nlohmann::json parsed_response;
	VKApi::auth_result auth;

	{
		std::string test_token = read_token();
		if (!test_token.empty())
		{
			auth = vk.auth_access_token(test_token);
		}
	}
	while (!auth.ok)
	{
		string_autoclean login;
		string_autoclean password;

		printf("Enter login: ");
		//std::getline(std::cin, login);
		login = read_token("login.txt");
		printf("Enter password: ");
		//read_password(password);
		password = read_token("password.txt");

		auth = vk.auth(login, password, VKApi::Device::PC);

		while (auth.need_2fa_sms)
		{
			printf("Enter 2fa SMS code: ");

			std::string code;
			std::getline(std::cin, code);

			auth = vk.auth_2fa(login, password, code);
		}

		while (auth.need_capcha)
		{
			url_open(auth.captcha_url.data());

			printf("%s\n\nEnter captcha value: ", auth.captcha_url.data());

			std::string captcha_value;
			std::getline(std::cin, captcha_value);

			auth = vk.auth_captcha(login, password, captcha_value);
		}

		while (auth.need_2fa_code)
		{
			printf("Enter 2fa code: ");

			std::string code;
			std::getline(std::cin, code);

			auth = vk.auth_2fa(login, password, code);
		}
	}

	{
		FILE* ftoken = fopen("token.txt", "w");
		if (ftoken)
		{
			fwrite(vk.access_token.data(), sizeof(char), vk.access_token.length(), ftoken);
			fclose(ftoken);
		}
	}


	response = vk.call("aada", {});
	try
	{
		parsed_response = nlohmann::json::parse(response);

	}
	catch (...)
	{

	}

	return 0;
}

int main(int argc, char** argv)
{
	int result = main2();
	getchar();
	return result;
}
