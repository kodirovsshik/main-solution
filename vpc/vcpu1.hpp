
#ifndef _VCPU1_HPP_
#define _VCPU1_HPP_


#include "bus.hpp"
#include "cpu.hpp"


class vcpu1_t :
	public cpu_t<uint16_t, uint8_t>
{
	bus_t<uint16_t, uint8_t>* bus;
	uint16_t pc;
	bool halt;

public:

	vcpu1_t() noexcept
	{
		this->reset();
	}


	void tie_bus(bus_t<uint16_t, uint8_t>& p)
	{
		this->bus = std::addressof(p);
	}
	void reset() noexcept
	{
		pc = 0;
		halt = false;
	}
	int tick()
	{
		if (this->halt) return 1;
		if (!this->bus) return 2;

		uint8_t op = this->bus->read(this->pc);
		int result = 0;
		int op_len = 0;

		switch (op)
		{
		case 0: //nop
			op_len = 1;
			break;

		case 1: //hlt
			this->halt = true;
			result = 1;
			break;

		default:
			result = 3;
		}

		this->pc += op_len;
		return result;
	}
};


#endif //!_VCPU1_HPP_
