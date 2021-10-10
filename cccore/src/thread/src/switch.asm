.intel_syntax noprefix

.global thread_switch_asm
thread_switch_asm:
	mov rsp, rdi //; First arg is pointer to saved CPU data
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi //; Ignore stack pointer by popping to junk reg for now
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax

	//; retq pops the rip from the stack and jumps there
	retq //; and we're off