.intel_syntax noprefix

.global thread_switch_asm
thread_switch_asm:
	mov rdi, rsp //; First arg is pointer to saved CPU data
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi //; Ignore stack pointer by popping to junk reg
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	mov rdi, [rsp] //; TODO: Remove

	addq rsp, 8   //; Don't leave the return address on the thread's stack
	jmpq [rsp] //; and we're off