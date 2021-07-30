
#ifndef _DIGILOG_GLOBALS_HPP_
#define _DIGILOG_GLOBALS_HPP_


#include <stdint.h>

#include <ksn/logger.hpp>


static constexpr size_t thread_buffer_size = 16384;
extern thread_local char thread_buffer[thread_buffer_size];

extern ksn::file_logger logger;



#endif //!_DIGILOG_GLOBALS_HPP_
