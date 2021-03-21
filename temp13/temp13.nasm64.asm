
global    main




section   .text


main: nop
	mov eax, 7
	mov ecx, 0
	cpuid 
	test ebx, 0b10000000000000000
	jnz .nret
	mov eax, -1
	ret
.nret:

	vpaddq zmm0, zmm1, zmm2
	vpaddq zmm0, zmm1, zmm2
	

	xor eax, eax
	ret

section   .data
