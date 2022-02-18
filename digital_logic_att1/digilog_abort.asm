
.const

ZERO LABEL FAR
_zero dq 0

.code

__digilog_abort proc
	jmp qword ptr [ZERO]
	jmp __digilog_abort
__digilog_abort endp

end
