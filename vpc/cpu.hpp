
#ifndef _CPU_HPP_
#define _CPU_HPP_


#include "bus.hpp"



template <class bus_ptr_t, class bus_data_t>
class cpu_t
{
public:
	bus_t<bus_ptr_t, bus_data_t>* bus;



	cpu_t()
	{
		this->reset();
	}


	
	virtual void tie_bus(bus_t<bus_ptr_t, bus_data_t>& b)
	{
		this->bus = std::addressof(b);
	}

	virtual void reset() {}
	//Returns execution status, 0 considered to be status "still going"
	virtual int tick() = 0;
	virtual ~cpu_t() = 0 {}
};


template <class P, class D>
int execute_vm(cpu_t<P, D>& cpu)
{
	int status;
	while ((status = cpu.tick()) == 0);
	return status;
}


#endif //!_CPU_HPP_