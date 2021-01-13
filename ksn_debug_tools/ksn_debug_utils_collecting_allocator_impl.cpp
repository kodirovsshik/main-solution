// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <ksn/debug_utils.hpp>

_KSN_BEGIN

std::list<ptrdiff_t> collecting_allocator_tracker::track_layers;

void collecting_allocator_tracker::track_start()
{
	collecting_allocator_tracker::track_layers.push_back(0);
}

ptrdiff_t collecting_allocator_tracker::track_stop()
{
	ptrdiff_t top = collecting_allocator_tracker::track_layers.back();
	collecting_allocator_tracker::track_layers.pop_back();
	return top;
}

_KSN_END