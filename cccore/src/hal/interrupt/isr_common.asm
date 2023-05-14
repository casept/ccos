.intel_syntax noprefix

.extern isr_disptach

//; FIXME: This assumes we were in userspace when interrupt arrived

.global isr_common
isr_common:
	//; Push all registers.
	//; This could probably be optimized further, depending on the exact ISR.
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rbp
	push rdi
	push rsi
	push rdx
	push rcx
	push rbx
	push rax

	//; Call C function that actually performs interrupt handler dispatch.
	//; This C function needs to be able to use arbitrary registers (for example, for syscall argument passing),
	//; as well as the interrupt number and argument.
	//; These have been stored on the stack by either the stub that called us or the CPU.
	mov rdi, rsp
	call isr_dispatch

	//; Restore all previously saved registers.
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop rbp
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15

	//; Remove argument and ISR number before returning.
	sub rsp, 2*8
	//; CPU will pop RIP, CS, RFLAGS, and SS and resume execution.
	iretq
