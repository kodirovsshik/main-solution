
bits 64


global _mm_and_native
global _mm_and_sse
global _mm_and_avx
global _mm_and_avx512


segment .code


	;RCX = dst
	;RDX = src1
	;R8 = src2
	;R9 = count in 64 byte blocks
_mm_and_sse:
	
	shl r9, 4 ; Now in 4 byte blocks
	jz .end

.loop:
	movaps xmm3, [rdx + r9 * 4 - 64 + 48]
	movaps xmm2, [rdx + r9 * 4 - 64 + 32]
	movaps xmm1, [rdx + r9 * 4 - 64 + 16]
	movaps xmm0, [rdx + r9 * 4 - 64 + 0]
	andps xmm3, [r8 + r9 * 4 - 64 + 48]
	andps xmm2, [r8 + r9 * 4 - 64 + 32]
	andps xmm1, [r8 + r9 * 4 - 64 + 16]
	andps xmm0, [r8 + r9 * 4 - 64 + 0]
	movaps [rcx + r9 * 4 - 64 + 48], xmm3
	movaps [rcx + r9 * 4 - 64 + 32], xmm2
	movaps [rcx + r9 * 4 - 64 + 16], xmm1
	movaps [rcx + r9 * 4 - 64 + 0], xmm0
	sub r9, 4 * 4
	jnz .loop
.end:

	ret
	

_mm_and_avx:
	mov r10, r9
	shr r9, 1; Now in 128 byte blocks
	shl r9, 5; Now in 4 byte blocks
	jz .end1
	
.loop:
	vmovupd ymm4, [rdx + r9 * 4 - 128 + 96]
	vmovupd ymm3, [rdx + r9 * 4 - 128 + 64]
	vmovupd ymm2, [rdx + r9 * 4 - 128 + 32]
	vmovupd ymm1, [rdx + r9 * 4 - 128 + 0]
	vandpd ymm4, [r8 + r9 * 4 - 128 + 96]
	vandpd ymm3, [r8 + r9 * 4 - 128 + 64]
	vandpd ymm2, [r8 + r9 * 4 - 128 + 32]
	vandpd ymm1, [r8 + r9 * 4 - 128 + 0]
	vmovupd [rcx + r9 * 4 - 128 + 96], ymm4
	vmovupd [rcx + r9 * 4 - 128 + 64], ymm3
	vmovupd [rcx + r9 * 4 - 128 + 32], ymm2
	vmovupd [rcx + r9 * 4 - 128 + 0], ymm1
	sub r9, 32
	jnz .loop
.end1:
	test r10b, r10b
	jnp .end
	shl r10, 3
	vmovupd ymm1, [rdx + r10 * 8 - 64 + 0]
	vmovupd ymm2, [rdx + r10 * 8 - 64 + 32]
	vandpd ymm1, [r8 + r10 * 8 - 64 + 0]
	vandpd ymm2, [r8 + r10 * 8 - 64 + 32]
	vmovupd [rcx + r10 * 8 - 64 + 0], ymm1
	vmovupd [rcx + r10 * 8 - 64 + 32], ymm2
.end:
	ret
	

_mm_and_avx512:
	mov r10, r9
	shr r9, 2; Now in 256 byte blocks
	shl r9, 5; Now in 8 byte blocks
	jz .end1
	
.loop1:
	vmovupd zmm4, [rdx + r9 * 8 - 256 + 192]
	vmovupd zmm3, [rdx + r9 * 8 - 256 + 64]
	vmovupd zmm2, [rdx + r9 * 8 - 256 + 64]
	vmovupd zmm1, [rdx + r9 * 8 - 256 + 0]
	vandpd zmm4, [r8 + r9 * 8 - 256 + 192]
	vandpd zmm3, [r8 + r9 * 8 - 256 + 128]
	vandpd zmm2, [r8 + r9 * 8 - 256 + 64]
	vandpd zmm1, [r8 + r9 * 8 - 256 + 0]
	vmovupd [rcx + r9 * 8 - 256 + 192], zmm4
	vmovupd [rcx + r9 * 8 - 256 + 128], zmm3
	vmovupd [rcx + r9 * 8 - 256 + 64], zmm2
	vmovupd [rcx + r9 * 8 - 256 + 0], zmm1
	sub r9, 32
	jnz .loop1
.end1:
	test r10b, 3
	jz .end
.loop2:
	dec r10b
	lea r11, [r10 * 8]
	vmovupd zmm4, [rdx + r11 * 8]
	vandpd zmm4, [r8 + r11 * 8]
	vmovupd [rcx + r11 * 8], zmm4
	test r10b, 3
	jnz .loop2
.end2:
.end:
	ret

	
	;RCX = dst
	;RDX = src1
	;R8 = src2
	;R9 = count in 64 byte blocks
_mm_and_native:
	push r12
	push r13
	push r14
	push r15
	push rsi
	push rdi

	shl r9, 3 ;now is a count of 8-byte blocks
	jz .end

.loop: 
	; do 32 bytes per iteration
	; 32 due to benchmark results
	mov r13, qword [rdx + r9 * 8 - 64 + 24]
	mov r12, qword [rdx + r9 * 8 - 64 + 16]
	mov r11, qword [rdx + r9 * 8 - 64 + 8]
	mov r10, qword [rdx + r9 * 8 - 64 + 0]
	and r13, qword [r8 + r9 * 8 - 64 + 24]
	and r12, qword [r8 + r9 * 8 - 64 + 16]
	and r11, qword [r8 + r9 * 8 - 64 + 8]
	and r10, qword [r8 + r9 * 8 - 64 + 0]
	mov qword [rcx + r9 * 8 - 64 + 24], r13
	mov qword [rcx + r9 * 8 - 64 + 16], r12
	mov qword [rcx + r9 * 8 - 64 + 8], r11
	mov qword [rcx + r9 * 8 - 64 + 0], r10
	sub r9, 4
	jnz .loop
	
.end:
	pop rdi
	pop rsi
	pop r15
	pop r14
	pop r13
	pop r12
	ret
