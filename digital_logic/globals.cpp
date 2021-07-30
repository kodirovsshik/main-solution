
#include "globals.hpp"
#include "window.hpp"
#include "graphics.hpp"
#include "opencl.hpp"





thread_local char thread_buffer[thread_buffer_size];


window_wrapper_t window;


draw_adapter_t draw_adapter;


cl_data_t cl_data;


ksn::file_logger logger;
