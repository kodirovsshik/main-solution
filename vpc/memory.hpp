
#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_


#include "bus.hpp"


template<size_t size, bool heap>
struct _memory_helper_storage
{
private:
	uint8_t* p;

public:
	_memory_helper_storage()
	{
		p = nullptr;
		p = new uint8_t[size];
		memset(p, 0, size);
	}
	~_memory_helper_storage() noexcept
	{
		delete[] p;
	}

	uint8_t* get_storage() const noexcept
	{
		return this->p;
	}
};

template<size_t size>
struct _memory_helper_storage<size, false>
{
private:
	uint8_t data[size] = { 0 };

public:
	uint8_t* get_storage() noexcept
	{
		return this->data;
	}
};


template<class ptr_t, ptr_t static_offset, bool is_dynamic>
struct _memory_helper_offset
{
	_memory_helper_offset(ptr_t) {}

	ptrdiff_t get_offset(ptr_t p) const noexcept
	{
		return ptrdiff_t(p - static_offset);
	}
};

template<class ptr_t, ptr_t static_offset>
struct _memory_helper_offset<ptr_t, static_offset, true>
{
	ptr_t dyn_off;
	_memory_helper_offset(ptr_t off) : dyn_off(off) {}

	ptrdiff_t get_offset(ptr_t p) const noexcept
	{
		return ptrdiff_t(p - this->dyn_off);
	}
};


template<
	class ptr_t,
	class data_t,
	size_t size,
	bool use_dynamic_offset = false,
	ptr_t static_offset = 0,
	bool writable = true,
	bool on_heap = false
>
class memory_t :
	public bus_t<ptr_t, data_t>::bus_device_t,
	public _memory_helper_storage<size, on_heap>,
	public _memory_helper_offset<ptr_t, static_offset, use_dynamic_offset>
{
public:

	memory_t(ptr_t dynamic_offset = 0) :
		_memory_helper_offset<ptr_t, static_offset, use_dynamic_offset>(dynamic_offset)
	{
	}


	virtual data_t read(ptr_t p)
	{
		return *this->get_pointer(p);
	}
	virtual void write(ptr_t p, data_t x)
	{
		if constexpr (writable)
		{
			*this->get_pointer(p) = x;
		}
	}


	data_t* get_pointer(ptr_t p)
	{
		return (data_t*)(this->get_storage() + this->get_offset(p));
	}
};

#endif //!_MEMORY_HPP_
