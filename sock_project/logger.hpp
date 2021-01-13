#pragma once

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <string>

#include <stdio.h>
#include <stdarg.h>
#include <vector>


class logger_t
{
public:

	std::vector<FILE*> files;
	std::string sender;

	void send(const char* p, size_t len = -1)
	{
		if (len == -1) len = strlen(p);
		
		for (auto file : files)
		{
			fwrite(p, sizeof(char), len, file);
		}
	}
	void send(const std::string& str)
	{
		this->send(str.data(), str.length());
	}
	
	void log(const char* type, const char* fmt, va_list ap)
	{
		this->send("[", 1);
		this->send(this->sender);
		this->send("][", 2);
		this->send(type);
		this->send("] ", 2);

		for (auto file : this->files)
		{
			vfprintf(file, fmt, ap);
		}
	}


	logger_t(const std::string& sender_name, FILE* to = stdout) : sender(sender_name), files(1, to) {}
	logger_t(const std::string& sender_name, std::initializer_list<FILE*> files) : sender(sender_name), files(files) 
	{
	}

	void info(const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		this->log("info", fmt, ap);
		va_end(ap);
	}
	void error(const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		this->log("error", fmt, ap);
		va_end(ap);
	}
	void append(FILE* p)
	{
		files.push_back(p);
	}
};

#endif //!_LOGGER_H_
