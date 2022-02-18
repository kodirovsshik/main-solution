
#ifndef _DIGILOG_WINDOW_H_
#define _DIGILOG_WINDOW_H_



#include <utility>

#include <ksn/stuff.hpp>
#include <ksn/window_gl.hpp>

#include "gl_switch.hpp"


struct window_wrapper_t
{
	std::pair<uint16_t, uint16_t> size = { 800, 600 };
	std::pair<uint16_t, uint16_t> pos;

#if DIGILOG_USE_OPENGL
	ksn::window_gl_t window;
#else
	ksn::window_t window;
#endif

};

extern window_wrapper_t window;



#endif //!_DIGILOG_WINDOW_H_
