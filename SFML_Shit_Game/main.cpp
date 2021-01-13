#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>
#include <Windows.h>
#include <list>
#include <chrono>
#include <thread>
#include <iostream>
#include <winres.h>
#include "resource.h"

#pragma warning(disable : 4996)
#pragma warning(disable : 4715)

sf::String russtr(const char* p)
{
	return sf::String(p, std::locale("Russian"));
}

int64_t KeyToSymbol(sf::Event::KeyEvent key)
{
	if (key.code <= sf::Keyboard::Z)
		if (key.shift)
			return (key.code + 65);
		else
			return (key.code + 97);
	else if (key.code >= sf::Keyboard::Num0 && key.code <= sf::Keyboard::Num9)
		if (key.shift)
			switch (key.code)
			{
			case 26: return U')';
			case 27: return U'!';
			case 28: return U'@';
			case 29: return U'#';
			case 30: return U'$';
			case 31: return U'%';
			case 32: return U'^';
			case 33: return U'&';
			case 34: return U'*';
			case 35: return U'(';
			}
		else
			return (key.code + 22);
	else if (key.code >= sf::Keyboard::Numpad0 && key.code <= sf::Keyboard::Numpad9)
		return key.shift ? 0 : key.code - 27;
	else if (key.code >= sf::Keyboard::LBracket && key.code <= sf::Keyboard::Space)
		switch (key.code)
		{
		case sf::Keyboard::LBracket: return key.shift ? U'{' : U'[';
		case sf::Keyboard::RBracket: return key.shift ? U'}' : U']';
		case sf::Keyboard::Semicolon: return key.shift ? U':' : U';';
		case sf::Keyboard::Comma: return key.shift ? U'<' : U',';
		case sf::Keyboard::Period: return key.shift ? U'>': U'.';
		case sf::Keyboard::Quote: return key.shift ? U'\"' : U'\'';
		case sf::Keyboard::Slash: return key.shift ? U'?' : U'/';
		case sf::Keyboard::Backslash: return key.shift ? U'|' : U'\\';
		case sf::Keyboard::Tilde: return key.shift ? U'~' : U'`';
		case sf::Keyboard::Equal: return key.shift ? U'=' : U'+';
		case sf::Keyboard::Hyphen: return key.shift ? U'_' : U'-';
		case sf::Keyboard::Space: return U' ';
		}
	else
		switch (key.code)
		{
		case sf::Keyboard::Add: return U'+';
		case sf::Keyboard::Subtract: return U'-';
		case sf::Keyboard::Multiply: return U'*';
		case sf::Keyboard::Divide: return U'/';
		case sf::Keyboard::Backspace: return U'\b';
		case sf::Keyboard::Enter: return U'\n';
		case sf::Keyboard::Escape: return -1;
		default: return U'\000';
		}
}

const std::chrono::duration<long long, std::ratio<1i64, 1000i64>> processor_sleep_time = std::chrono::milliseconds(20);

int game(sf::RenderWindow& window)
{
	TCHAR fontPath[MAX_PATH];
	UINT a = GetWindowsDirectoryA(fontPath, MAX_PATH);
	sprintf(fontPath + a, "%s", "\\Fonts\\Arial.ttf");
	sf::Font font;
	if (!font.loadFromFile(fontPath))
	{
		MessageBoxA(0, "Failed to load system arial font", "Error", MB_ICONERROR);
		return 1;
	}

	HRSRC hres = FindResourceA(0, MAKEINTRESOURCE(IDB_PNG1), "PNG");
	HANDLE hmem = LoadResource(0, hres);
	HANDLE tileset = LockResource(hmem);
	DWORD tilesize = SizeofResource(0, hres);

	sf::Texture texture;
	if (!texture.loadFromMemory(tileset, tilesize))
	{
		MessageBoxA(0, "Low memory, failed to load resources", "Error", MB_ICONERROR);
		return 2;
	}
	FreeResource(hmem);
	sf::Sprite car1(texture, sf::IntRect(0, 0, 21, 36));
	sf::Sprite car2(texture, sf::IntRect(21, 0, 24, 33));
	sf::Sprite car3(texture, sf::IntRect(45, 0, 21, 51));
	sf::Sprite car4(texture, sf::IntRect(66, 0, 27, 54));
	sf::Sprite car5_1(texture, sf::IntRect(93, 0, 21, 51));
	sf::Sprite car5_2(texture, sf::IntRect(114, 0, 21, 51));
	sf::Sprite car6(texture, sf::IntRect(135, 0, 42, 42));

	sf::Sprite menu1_1(texture, sf::IntRect(0, 54, 280, 57));
	sf::Sprite menu1_2(texture, sf::IntRect(0, 111, 280, 57));
	sf::Sprite menu2_1(texture, sf::IntRect(0, 167, 309, 56));
	sf::Sprite menu2_2(texture, sf::IntRect(0, 224, 309, 56));

	sf::Event ev;

	sf::Sprite* menubutton1;
	sf::Sprite* menubutton2;

	menu1_1.setPosition(200, 150);
	menu1_2.setPosition(200, 160);
	menu2_1.setPosition(200, 220);
	menu2_2.setPosition(200, 230);

menu:

	int selected = 0;
	while (selected == 0)
	{
		if (!window.isOpen())
		{
			window.close();
			return 0;
		}
		menubutton1 = &menu1_1;
		menubutton2 = &menu2_1;

		if (sf::IntRect(200, 160, 280, 57).contains(sf::Mouse::getPosition(window)))
			menubutton1 = &menu1_2;
		if (sf::IntRect(200, 230, 309, 56).contains(sf::Mouse::getPosition(window)))
			menubutton2 = &menu2_2;

		while (window.pollEvent(ev))
		{
			if (ev.type == sf::Event::MouseButtonPressed)
			{
				if ((ev.mouseButton.button == sf::Mouse::Button::Left) && (menubutton1 == &menu1_2))
				{
					selected = 1;
					break;
				}
				else if ((ev.mouseButton.button == sf::Mouse::Button::Left) && (menubutton2 == &menu2_2))
				{
					selected = 2;
					break;
				}
			}
			else if (ev.type == sf::Event::Closed)
			{
				window.close();
				return 0;
			}
		}

		window.clear({ 50, 50, 50, 255 });
		window.draw(*menubutton1);
		window.draw(*menubutton2);
		window.display();
		std::this_thread::sleep_for(processor_sleep_time);
	}

	if (selected == 1)
	{
		int creating = 0;
		sf::Text text_enter_port(L"¬ведите порт:", font, 48);
		sf::Text text_port(L"", font, 48);
		short port;

		text_enter_port.setPosition(100, 150);
		text_port.setPosition(420, 150);
		
		while (creating == 0)
		{
			if (!window.isOpen())
				return 0;
			while (window.pollEvent(ev))
			{
				if (ev.type == sf::Event::EventType::KeyPressed)
				{
					bool a = false;
					a |= ev.key.alt;
					a |= ev.key.control;
					a |= ev.key.shift;
					a |= ev.key.system;
					if (!a)
					{
						if (ev.key.code == sf::Keyboard::Escape)
							text_port.setString(L"");
						else if ((ev.key.code >= sf::Keyboard::Num0) && (ev.key.code <= sf::Keyboard::Num9))
						{
							std::basic_string<sf::Uint32> str = text_port.getString().toUtf32();

							if (str.size() < 5)
							{
								if (!((str.size() == 0) && (ev.key.code == sf::Keyboard::Num0)))
								{
									str.push_back(ev.key.code + 22);
									text_port.setString(str);
								}
							}
						}
						else if ((ev.key.code >= sf::Keyboard::Numpad0) && (ev.key.code <= sf::Keyboard::Numpad9))
						{
							std::basic_string<sf::Uint32> str = text_port.getString().toUtf32();

							if (str.size() < 5)
							{
								if (!((str.size() == 0) && (ev.key.code == sf::Keyboard::Numpad0)))
								{
									str.push_back(ev.key.code - 27);
									text_port.setString(str);
								}
							}
						}
						else if ((ev.key.code == sf::Keyboard::Return) && text_port.getString().getSize())
						{
							creating = 1;
							std::string str = text_port.getString().toAnsiString();
							sscanf(str.data(), "%hu", &port);
						}
						else if (ev.key.code == sf::Keyboard::BackSpace)
						{
							std::basic_string<sf::Uint32> str = text_port.getString().toUtf32();

							if (str.size())
							{
								str.erase(str.size() - 1, 1);
								text_port.setString(str);
							}
						}
					} //!a
				}//key pressed
			}//poll event
			
			window.clear({ 50, 50, 50, 255 });
			window.draw(text_enter_port);
			window.draw(text_port);
			window.display();
		}//creating == 0

		sf::Text text_enter_nickname(L"¬ведите никнейм:", font, 48);
		sf::Text text_name(L"", font, 36);
		text_enter_nickname.setPosition(100, 210);
		text_name.setPosition(100, 270);

		while (creating == 1)
		{
			if (!window.isOpen())
				return 0;
			while (window.pollEvent(ev))
			{
				if (ev.type == sf::Event::KeyPressed)
				{
					bool err = false;
					err |= ev.key.alt;
					err |= ev.key.system;
					err |= ev.key.control;
					if (err)
						continue;

					char32_t symbol = KeyToSymbol(ev.key);
					switch (symbol)
					{
					case 0: break;
					case '\b':
					{
						sf::String str = text_name.getString();
						size_t size = str.getSize();
						if (size)
						{
							str.erase(size - 1, 1);
							text_name.setString(str);
						}
						break;
					}
					case -1:
						text_name.setString(L"");
						break;
					default:
					{
						sf::String str = text_name.getString();
						if (str.getSize() < 30)
						{
							str += symbol;
							text_name.setString(str);
						}
						break;
					}
					case '\n':
						creating = 2;
					}
				}
			}
			window.clear({ 50, 50, 50, 255 });
			window.draw(text_enter_port);
			window.draw(text_port);
			window.draw(text_enter_nickname);
			window.draw(text_name);
			window.display();
			std::this_thread::sleep_for(processor_sleep_time);
		}

		sf::Text text_enter_players(L"¬ведите количество игроков: ", font, 48);
		sf::Text text_players(L"", font, 48);
		text_enter_players.setPosition(100, 320);
		text_players.setPosition(100, 380);

		sf::TcpListener server;
		while (creating == 2)
		{
			if (!window.isOpen())
				return 0;
			while (window.pollEvent(ev))
			{
				if (ev.type == sf::Event::EventType::KeyPressed)
				{
					bool a = false;
					a |= ev.key.alt;
					a |= ev.key.control;
					a |= ev.key.shift;
					a |= ev.key.system;
					if (!a)
					{
						if (ev.key.code == sf::Keyboard::Escape)
							text_players.setString(L"");
						else if ((ev.key.code >= sf::Keyboard::Num0) && (ev.key.code <= sf::Keyboard::Num9))
						{
							std::basic_string<sf::Uint32> str = text_players.getString().toUtf32();

							if (str.size() < 1)
							{
								if (!((str.size() == 0) && (ev.key.code == sf::Keyboard::Num0)))
								{
									str.push_back(ev.key.code + 22);
									text_players.setString(str);
								}
							}
						}
						else if ((ev.key.code >= sf::Keyboard::Numpad0) && (ev.key.code <= sf::Keyboard::Numpad9))
						{
							std::basic_string<sf::Uint32> str = text_players.getString().toUtf32();

							if (str.size() < 1)
							{
								if (!((str.size() == 0) && (ev.key.code == sf::Keyboard::Numpad0)))
								{
									str.push_back(ev.key.code - 27);
									text_players.setString(str);
								}
							}
						}
						else if ((ev.key.code == sf::Keyboard::Return) && text_players.getString().getSize())
						{
							std::string str = text_players.getString().toAnsiString();
							if (str != "1")
								creating = 3;
						}
						else if (ev.key.code == sf::Keyboard::BackSpace)
						{
							std::basic_string<sf::Uint32> str = text_players.getString().toUtf32();

							if (str.size())
							{
								str.erase(str.size() - 1, 1);
								text_players.setString(str);
							}
						}
					} //!a
				}//key pressed
			}//poll event
			window.clear({ 50, 50, 50, 255 });
			window.draw(text_enter_port);
			window.draw(text_port);
			window.draw(text_enter_nickname);
			window.draw(text_name);
			window.draw(text_enter_players);
			window.draw(text_players);
			window.display();
			std::this_thread::sleep_for(processor_sleep_time);
		}//creating == 2

		uint8_t players;
		sscanf((char*)text_players.getString().toUtf8().data(), "%hhi", &players);
		sf::Text text_lobby(L"Ћобби", font, 50);
		sf::Text playernames[9];
		sf::TcpSocket sockets[9];

		text_lobby.setPosition(300, 50);
		while (creating == 3)
		{
			window.clear({50, 50, 50, 255});
			window.draw(text_lobby);
			window.display();
		}

	}//create a server (selected in main menu)

	MessageBoxA(window.getSystemHandle(), "Windows detected that your penis is small, program will be terminated", "Error", MB_ICONERROR);
	window.close();
	std::cout << selected;
	std::cin.get();
	return 0;
}

#if 0
int main()
#else
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
#endif
{
	int res;
	sf::RenderWindow window(sf::VideoMode(800, 600), "Ti pidor", sf::Style::Close);
	while ((res = game(window)) == -1);
	return res;
}