const char* test_prog = R"(

__kernel void iota(__global int *ptr, __global int *args)
{
	//args:
	//[0] = init value
	//[1] = step
	int id = get_global_id(0);
	ptr[id] = args[0] * id + args[1]
}

__kernel void process(__global int* ptr)
{
	__global int* a = ptr + get_global_id(0);
	*a = *a * 2 + 1;
}

)";
