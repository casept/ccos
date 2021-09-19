.intel_syntax noprefix

.section .text
.global interrupt_default_isr
interrupt_default_isr:
	cli

	// No need to save registers or anything, we hang after this anyways
	call interrupt_default_isr_c

	interrupt_default_isr_spin:
		jmp interrupt_default_isr_spin