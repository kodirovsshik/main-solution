
#include "vcpu1.hpp"
#include "memory.hpp"


int main()
{

	bus_t<uint16_t, uint8_t> bus;
	memory_t<uint16_t, uint8_t, 65536, false, 0, true, false> ram;
	vcpu1_t cpu;

	bus.connect(ram, 0, 0xFFFF);
	cpu.tie_bus(bus);

	ram.get_storage()[3] = 1;

	return execute_vm(cpu);
}