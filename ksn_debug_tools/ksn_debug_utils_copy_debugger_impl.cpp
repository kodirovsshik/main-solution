// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <ksn/debug_utils.hpp>

_KSN_BEGIN

FILE* copy_debugger::log_file = stderr;
bool copy_debugger::force_debug_breaks = false;
bool copy_debugger::break_at_check_error = false;

#pragma warning(disable : 26439)

copy_debugger::copy_debugger()
{
	fprintf(copy_debugger::log_file, "[COPY DEBUGGER] constructor() at %p\n", this);
	this->signature = 0xDEADC0DE;

	if (copy_debugger::force_debug_breaks)
		copy_debugger::_break();
}

copy_debugger::copy_debugger(const copy_debugger& other)
{
	fprintf(copy_debugger::log_file, "[COPY DEBUGGER] constructor(const ref to %p) at %p\n", &other, this);
	other.check();
	this->signature = 0xDEADC0DE;

	if (copy_debugger::force_debug_breaks)
		copy_debugger::_break();
}

copy_debugger::copy_debugger(copy_debugger&& other)
{
	fprintf(copy_debugger::log_file, "[COPY DEBUGGER] constructor(rvalue ref to %p) at %p\n", &other, this);
	other.check();
	this->signature = 0xDEADC0DE;

	if (copy_debugger::force_debug_breaks)
		copy_debugger::_break();
}

copy_debugger::~copy_debugger()
{
	fprintf(copy_debugger::log_file, "[COPY DEBUGGER] destructor() at %p\n", this);
	this->check();
	this->signature = 0;

	if (copy_debugger::force_debug_breaks)
		copy_debugger::_break();
}

copy_debugger& copy_debugger::operator=(const copy_debugger& other)
{
	fprintf(this->log_file, "[COPY DEBUGGER] operator=(const& to %p) at %p\n", &other, this);
	other.check();

	if (copy_debugger::force_debug_breaks)
		copy_debugger::_break();

	return *this;
}

copy_debugger& copy_debugger::operator=(copy_debugger&& other)
{
	fprintf(this->log_file, "[COPY DEBUGGER] operator=(&& to %p) at %p\n", &other, this);
	other.check();
	//other.signature = 0;

	if (copy_debugger::force_debug_breaks)
		copy_debugger::_break();
	
	return *this;
}

void copy_debugger::log() const
{
	fprintf(this->log_file, "[COPY DEBUGGER] Log() at %p\n", this);
	this->check();

	if (copy_debugger::force_debug_breaks)
		copy_debugger::_break();
}

void copy_debugger::check() const
{
	if (this->signature != 0xDEADC0DE)
	{
		fprintf(this->log_file, "[COPY DEBUGGER] CHECK ERROR AT %p\n\a", this);
		if (copy_debugger::break_at_check_error)
			copy_debugger::_break();
	}
}

_KSN_END