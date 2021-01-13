.model flat

.code

??Rformula@@QAANZZ proc
	push ebp
	mov ebp, esp


	mov ecx, dword ptr [ebp + 12]

	lea eax, dword ptr [ebp + 16]
	push eax

	call ?solve@formula@@QAENPAD@Z


	mov esp, ebp
	pop ebp

	ret
??Rformula@@QAANZZ endp


end