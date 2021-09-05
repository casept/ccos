.intel_syntax noprefix
.code16
.section .boot1.text,"ax"
.global _boot1_start
_boot1_start:
	jmp boot1

# Stack (for while we're in real mode)
boot1_stack: .fill 128, 0x00
boot1_stack_top = .
# GDT data
.align 8
gdt:
	# Null segment descriptor (required by HW)
	.byte 0x00
	.byte 0x00
	.byte 0x00
	.byte 0x00
	.byte 0x00
	.byte 0x00
	.byte 0x00
	.byte 0x00
	# 4GB code segment
	.byte 0xFF       # Segment limit bits 0-7
	.byte 0xFF       # Segment limit bits 8-15
	.byte 0x00       # Base address bits 0-7
	.byte 0x00       # Base address bits 8-15
	.byte 0x00       # Base address bits 16-23
	.byte 0b10011010 # Readable, nonconforming, code, Ring 0, present
	.byte 0b11001111 # 32-bit code, rough granularity, bits 16-19 of segment limit
	.byte 0x00       # Base address bits 24-31
	# 4GB data segment
	.byte 0xFF       # Segment limit bits 0-7
	.byte 0xFF       # Segment limit bits 8-15
	.byte 0x00       # Base address bits 0-7
	.byte 0x00       # Base address bits 8-15
	.byte 0x00       # Base address bits 16-23
	.byte 0b10010010 # Writable, expand downward (though direction is probably irrelevant here), data, Ring 0, present
	.byte 0b11001111 # big, rough granularity, segment limit bits 16-19
	.byte 0x00       # Base address bits 24-31
gdt_end:
.align 8
gdt_desc:
	.short gdt_end - gdt
	.int gdt

# Various strings
boot1_welcome_message: .asciz "boot1: running\r\n"
boot1_exit_message:    .asciz "boot1: entering protected mode\r\n"


# Copied here because I'm not sure calling boot0 from boot1 is such a good idea.
.global boot1_bios_print
boot1_bios_print:
	push ax

	cld
	boot1_bios_print_loop:
		lodsb
		cmp al, 0
		je boot1_bios_print_done

		call boot1_bios_putchar
		jmp boot1_bios_print_loop

	boot1_bios_print_done:
		pop ax
		ret

.global boot1_bios_putchar
boot1_bios_putchar:
	push ax
	push bx

	mov ah, 0x0E
	mov bh, 0x00
	mov bl, 0xFF
	int 0x10

	pop bx
	pop ax
	ret


boot1:
	# Re-initialize segment registers, just to be safe
	mov ax, 0x00
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	# Initialize a new stack for us
	mov sp, boot1_stack_top

	# Announce progress
	push si
	lea si, boot1_welcome_message
	call boot1_bios_print
	pop si

	# TODO: Ask BIOS whether we have enough lower/upper RAM

	call boot1_ensure_a20_active

	# Final GDT load and escape to protected mode
	lgdt [gdt_desc]
	mov ebx, boot1_cmain
	jmp enter_protected_mode


enter_protected_mode:
	# Disable interrupts
	cli
	# TODO: Disable NMI

	# Say goodbye
	push si
	lea si, boot1_exit_message
	call boot1_bios_print
	pop si

	# Enable protection
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	# Immediately far jump into 32-bit code
	jmp 0x08:moar_bits
	.code32
	moar_bits:

	# Code segment descriptor is already correct due to above far jump
	# We want data segment descriptor #2
	mov ax, 2*8 # Value is multiplied by 8 because that's the size of a segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	# Create yet another stack right below where we copied the kernel to
	mov esp, 0x0000f000

	# TODO: Hand off to whatever address is in ebx
	# jmp ebx
	jmp boot1_cmain
