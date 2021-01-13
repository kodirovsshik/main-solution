.model flat, C

.code

get_base_pointer proc 
	mov eax, ebp
	ret
get_base_pointer endp

_stacktrace1 proc
	xor ecx, ecx

	mov eax, dword ptr [esp + 12] ; eax = ptr to counter
	test eax, eax
	jz @ret

	mov dword ptr [eax], ecx

	mov ecx, dword ptr [esp + 8] ; ecx = left buffer entries number
	test ecx, ecx ;if ecx = 0, then return 0
	jz @ret

	mov edx, dword ptr [esp + 4] ; edx = buffer pointer
	test edx, edx ;if edx = nullptr, then return 0
	jz @ret

	push ebx ; To be used as a temp variable
	push ebp ; To be used as a current frame pointer
	mov ebp, dword ptr [ebp] ; Discard current function
	mov ebp, dword ptr [ebp] ; Discard exception wrapper
@loop_:
	test ebp, ebp
	jz @loop_out
	inc dword ptr [eax]
	mov dword ptr [edx], ebp
	mov ebp, dword ptr [ebp]
	add edx, 4
	loop @loop_
@loop_out:

	pop ebp
	pop ebx
@ret:
	ret
_stacktrace1 endp

end
