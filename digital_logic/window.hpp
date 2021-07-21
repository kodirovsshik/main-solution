
#ifndef _DIGILOG_WINDOW_H_
#define _DIGILOG_WINDOW_H_



#include <utility>

#include <ksn/stuff.hpp>
#include <ksn/window.hpp>


struct window_wrapper_t
{
	std::pair<uint16_t, uint16_t> size = { 800, 600 };
	std::pair<uint16_t, uint16_t> pos;

	ksn::window_t window;

};

extern window_wrapper_t window;



#endif //!_DIGILOG_WINDOW_H_
