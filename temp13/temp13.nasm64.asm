
global    main

section   .text
main:   
	xor rax, rax
	ret

section   .data
message:  db        "Hello, World", 10      ; note the newline at the end
