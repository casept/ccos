.intel_syntax noprefix

.macro push_interrupt
push rax
push rcx
push rdx
push rbx
push rbp
push rsi
push rdi
.endm

.macro pop_interrupt
pop rdi
pop rsi
pop rbp
pop rbx
pop rdx
pop rcx
pop rax
.endm

.macro push_c_caller_saves
push r10
push r11
.endm

.macro pop_c_caller_saves
pop r11
pop r10
.endm

.section .text
.global pit_isr
.section .text


.global exception_dbz
exception_dbz:
	cli // TODO: This is insufficient for multiprocessor
	push_interrupt

	push_c_caller_saves
	# TODO: Save errcode (is at top of stack)
	push rdi
	push rsi
	mov rdi, 0 # Exception ID 0
	mov rsi, 0 # Dummy error code (this exception doesn't have one)
	call exception_generic_handler_c
	pop rsi
	pop rdi
	pop_c_caller_saves

	pop_interrupt
	sti
	iretq
