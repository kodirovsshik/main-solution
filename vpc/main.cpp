
#include "vcpu1.hpp"
#include "memory.hpp"


int main()
{

	bus_t<uint16_t, uint8_t> bus;
	uint8_t ram_storage[0xFFFF];
	memory_t<uint16_t, uint8_t> ram(ram_storage);
	vcpu1_t cpu;

	ram_storage[0] = 1;

	bus.connect(ram, 0, 0xFFFF);
	cpu.tie_bus(bus);

	return execute_vm(cpu);
}