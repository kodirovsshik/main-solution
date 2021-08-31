
#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_


#include "bus.hpp"



template<
	class ptr_t,
	class data_t,
	bool writable = true
>
class memory_t :
	public bus_t<ptr_t, data_t>::bus_device_t
{
	uint8_t* m_storage = nullptr;


public:

	memory_t() {}
	memory_t(void* storage)
		: m_storage((uint8_t*)storage)
	{
	}

	virtual data_t read(ptr_t p) const noexcept
	{
		return *this->get_pointer(p);
	}
	virtual void write(ptr_t p, data_t x) noexcept
	{
		if constexpr (writable)
		{
			*this->get_pointer(p) = x;
		}
	}


	data_t* get_pointer(ptr_t p) const noexcept
	{
		return (data_t*)(this->m_storage + (intptr_t)p);
	}

	template<class return_ptr_t = data_t>
	return_ptr_t* get_storage() const noexcept
	{
		return (return_ptr_t*)this->m_storage;
	}
	void set_storage(void* storage) noexcept
	{
		this->m_storage = (uint8_t*)storage;
	}
};

#endif //!_MEMORY_HPP_
