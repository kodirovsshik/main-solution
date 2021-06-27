
#ifndef _BUS_HPP_
#define _BUS_HPP_


#include <vector>
#include <concepts>
#include <algorithm>
#include <ranges>





template<typename T>
concept bus_pointer = std::integral<T> || std::is_pointer_v<T>;



template<bus_pointer ptr_t, std::integral data_t>
class bus_t
{
public:

	class bus_device_t
	{
	public:
		virtual ~bus_device_t() = 0 {};

		virtual data_t read(ptr_t) = 0;
		virtual void write(ptr_t, data_t) = 0;
	};



private:

	struct bus_device_entry_t
	{
		bus_device_t* m_device;
		ptr_t begin, end;

		friend bool operator<(const bus_device_entry_t& lhs, const bus_device_entry_t& rhs) noexcept
		{
			return lhs.begin < rhs.begin;
		}
	};

	std::vector<bus_device_entry_t> m_devices;



public:

	bus_t() {}

	virtual ~bus_t() {}


	data_t read(ptr_t addr)
	{
		for (auto iter = this->m_devices.rbegin(); iter != this->m_devices.rend(); ++iter)
		{
			if (addr >= iter->begin && addr <= iter->end)
				return iter->m_device->read(addr);
		}
		return data_t(0);
	}
	void write(ptr_t addr, data_t data)
	{
		for (auto iter = this->m_devices.rbegin(); iter != this->m_devices.rend(); ++iter)
		{
			if (addr >= iter->begin && addr <= iter->end)
				return iter->m_device->write(addr, data);
		}
	}



	void connect(bus_device_t& d, ptr_t begin, ptr_t end)
	{
		this->m_devices.push_back({ std::addressof(d), begin, end });
	}
	void disconnect(bus_device_t& d)
	{
		for (auto iter = this->m_devices.begin(); iter != this->m_devices.end(); )
		{
			if (iter->m_device == std::addressof(d))
				iter = this->m_devices.erase(iter);
			else
				++iter;
		}
	}



private:
};


#endif //!_BUS_HPP_