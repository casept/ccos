.intel_syntax noprefix
.code16
.section .boot1.text,"ax"

# This file provides an implementation of BIOS-supported
# kernel copying from the floppy into the beginning of extended memory.
# To support this, the code always reads exactly one track,
# switches to protected mode, copies the track there, then goes back to real mode.

.global boot1_copy_kernel
boot1_copy_kernel:
	push ax
	push bx
	push cx
	push dx

	boot1_copy_kernel_loop:
		# Increment retry counter
		mov ax, [boot1_floppy_read_retry_counter]
		inc ax
		mov [boot1_floppy_read_retry_counter], ax

		# TODO: Work on double-sided floppy (or directly HDD support)
		# mov al, 18                    # Read entire cylinder at once
		mov al, 2
		mov ch, [boot1_next_cylinder] # Which cylinder
		mov cl, 1                     # Start at cylinder's first sector (indexing seems to be 1-based)
		mov dh, 0                     # For now, only use 0th head
		# dl already set to boot drive by BIOS
		mov bx, [boot1_kernel_copy_low]

		# Call
		mov ah, 0x02
		int 0x13
		jc boot1_floppy_read_error

		# Success; reset retry counter
		mov ax, 0
		mov [boot1_floppy_read_retry_counter], ax

		# Move to next cylinder
		mov bx, [boot1_next_cylinder]
		inc bx
		mov [boot1_next_cylinder], bx

		jmp boot1_copy_kernel_done # TODO: Remove

		jmp boot1_copy_kernel_loop

	boot1_copy_kernel_done:
		pop dx
		pop cx
		pop bx
		pop ax
		ret

	boot1_floppy_read_error:
		# Retry counter exhausted for this read?
		push ax
		mov ax, [boot1_floppy_read_retry_counter]
		cmp ax, 3 # 3 retries allowed
		jle boot1_copy_kernel_loop
		pop ax

		# Exhausted; print error and hang
		lea si, boot1_floppy_read_error_message
		call boot1_bios_print
		# Error code is in ah, but putchar expects it in al
		mov al, ah
		add al, 48 # Number to ASCII
		call boot1_bios_putchar
		boot1_floppy_read_error_spin:
			jmp boot1_floppy_read_error_spin

	boot1_next_cylinder:             .short 1
	boot1_floppy_read_retry_counter: .byte 0x00, 0x00
	boot1_floppy_read_error_message: .asciz "boot1: failed to read kernel from floppy, error code: "
	# Low memory where chunks of the kernel are copied into by BIOS
	boot1_kernel_copy_low:   .short 0xF000
	# Extended memory where the kernel image is finally copied to
	#boot1_kernel_copy_extended:  0x00100000



# Adapted from https://wiki.osdev.org/Real_Mode#Switching_from_Protected_Mode_to_Real_Mode

idt_real:
	.word 0x3ff		 # 256 entries, 4b each = 1K
	.byte 0x00, 0x00, 0x00, 0x00 # Real Mode IVT @ 0x0000

savcr0:
	.byte 0x00, 0x00, 0x00, 0x00 # Storage location for pmode CR0.

boot1_real_protected:
    # We are already in 16-bit mode here!
	cli

	# Need 16-bit Protected Mode GDT entries!
	mov eax, 0 # 16-bit Protected Mode data selector.
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	mov ss, eax


	# Disable paging (we need everything to be 1:1 mapped).
	mov eax, cr0
	mov [savcr0], eax	# save pmode CR0
	and eax, 0x7FFFFFFe	# Disable paging bit & disable 16-bit pmode.
	mov cr0, eax

	jmp 0x00:fewer_bits	# Perform Far jump to set CS.

fewer_bits:
	mov sp, 0x8000	# pick a stack pointer.
	mov ax, 0		# Reset segment registers to 0.
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	lidt [idt_real]
	sti			# Restore interrupts -- be careful, unhandled int's will kill it.
