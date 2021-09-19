.intel_syntax noprefix

.section .text
.global pit_isr
pit_isr:
	cli // TODO: This is insufficient for multiprocessor

	// Registers not listed here are saved by the C compiler once below C function call happens
	// Segment registers are also not saved, because the OS doesn't support 32-bit executables
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11

	call pit_isr_c // No args, no return value

	pop r11
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	pop rax

	sti
	iretq
