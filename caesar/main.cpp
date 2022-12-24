
#pragma warning(disable : 4005 5106)

import <iostream>;
import <string>;

import <clocale>;

import <windows.h>;
import <conio.h>;


using namespace std;

void fill_tables();
void encrypt();
void decrypt();
void pause();
istream& endline(istream& is);
wstring widen(const string& s);


int main()
{
	setlocale(0, "ru");
	SetConsoleCP(1251);
	fill_tables();

	while (true)
	{
		system("cls");
		wcout << L" Меню:\n";
		wcout << L" 1 - Зашифровка текста\n";
		wcout << L" 2 - Расшифровка текста\n";
		wcout << L" 0 - Выход\n";
		wcout << L" >>> ";
		int x = 0;
		cin >> x >> endline;
		

		if (x == 1)
			encrypt();
		else if (x == 2)
			decrypt();
		else
			return 0;
		pause();
	}
	return 0;
}



uint8_t table_key_period[256]{};
char table_next_char[256]{};

void fill_tables()
{
	for (int i = 0; i < 256; ++i)
	{
		table_key_period[i] = 1;
		table_next_char[i] = 0;
	}

	for (int i = 'a'; i <= 'z'; ++i)
	{
		table_key_period[i] = 26;
		table_next_char[i] = i + 1;
	}
	for (int i = 'A'; i <= 'Z'; ++i)
	{
		table_key_period[i] = 26;
		table_next_char[i] = i + 1;
	}

	for (int i = (unsigned char)'а'; i <= (unsigned char)'я'; ++i)
	{
		table_key_period[i] = 33;
		table_next_char[i] = i + 1;
	}
	for (int i = (unsigned char)'А'; i <= (unsigned char)'Я'; ++i)
	{
		table_key_period[i] = 33;
		table_next_char[i] = i + 1;
	}

	table_next_char['z'] = 'a';
	table_next_char['Z'] = 'A';

	table_next_char[(unsigned char)'я'] = 'а';
	table_next_char[(unsigned char)'Я'] = 'А';

	table_next_char[(unsigned char)'е'] = 'ё';
	table_next_char[(unsigned char)'Е'] = 'Ё';

	table_next_char[(unsigned char)'ё'] = 'ж';
	table_next_char[(unsigned char)'Ё'] = 'Ж';

	table_key_period[(unsigned char)'ё'] = 33;
	table_key_period[(unsigned char)'Ё'] = 33;
}

void update(char& c, int key)
{
	int mod = table_key_period[(unsigned char)c];
	
	key %= mod;
	if (key < 0) key += mod;

	for (int i = 0; i < key; ++i)
		c = table_next_char[(unsigned char)c];
}
void update(string& s, int key)
{
	for (char& c : s)
		update(c, key);
}



void pause()
{
	wcout << L"Нажмите Enter для продолжения";
	while (_getch() != 13);
}

void encrypt()
{
	string s;
	int k = 0;

	wcout << L"Введите строку: ";
	getline(cin, s);

	wcout << L"Введите ключ: ";
	cin >> k;

	update(s, k);

	wcout << L"Зашифрованная строка: " << widen(s) << endl;
	pause();
}
void decrypt()
{
	wcout << L"Введите строку: ";

	string s;
	getline(cin, s);

	int k = 1;
	for (char c : s)
	{
		int ck = table_key_period[(unsigned char)c];
		if (k != 1 && ck != 1 && ck != k)
		{
			wcout << L"Ошибка: сообщение должно состоять из алфавитов одинаковой мощности\n";
			return;
		}
		if (ck != 1)
			k = ck;
	}
	wcout << L"Варианты расшифровки:\n";
	for (int i = 0; i < k; ++i)
	{
		wcout.precision(2);
		wcout << i << L": " << widen(s) << endl;
		update(s, 1);
	}
}


istream& endline(istream& is)
{
	is.ignore(INT_MAX, '\n');
	return is;
}
wstring widen(const string& s)
{
	wstring r;
	r.resize(s.size() + 1);
	MultiByteToWideChar(1251, 0, s.data(), (int)s.size(), r.data(), (int)r.size());
	return r;
}