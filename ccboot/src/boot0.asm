.intel_syntax noprefix
.code16
.section .boot0.text,"ax"
.global _boot0_start
_boot0_start:
	cli
	cld
	jmp boot0

# Misc. data
boot0_error_message:   .asciz "boot0: failed, code: "
boot0_welcome_message: .asciz "boot0: running\r\n"
boot0_exit_message:    .asciz "boot0: done\r\n"

# Reserve a tiny stack for this stage
boot0_stack: .fill 32, 0x00
boot0_stack_top = .

# Print the NULL-terminated string, whose address is passed in si.
# Has to be implemented manually because I can't get the BIOS impl working.
boot0_bios_print:
	push ax

	boot0_bios_print_loop:
		lodsb
		cmp al, 0
		je boot0_bios_print_done

		call boot0_bios_putchar
		jmp boot0_bios_print_loop

	boot0_bios_print_done:
		pop ax
		ret

boot0_bios_putchar:
	push ax
	push bx

	mov ah, 0x0E
	mov bh, 0x00
	mov bl, 0xFF
	int 0x10

	pop bx
	pop ax
	ret

# Error handling function.
# Prints the hardcoded error message,
# followed by the ASCII error code passed in al.
boot0_error:
	lea si, boot0_error_message
	call boot0_bios_print
	call boot0_bios_putchar

	boot0_error_spin:
		jmp boot0_error_spin


boot0_floppy_reset_error:
	mov al, 'A'
	jmp boot0_error

boot0_floppy_read_error:
	mov al, 'B'
	jmp boot0_error


.set BOOT1_START_ADDR, 0x1000

# Load the second stage bootloader (boot1),
# located immediately after the boot sector
boot0:

	# Ensure that we have a canonical code segment and instruction pointer,
	# because some BIOSes load the boot block in a different segment.
	ljmp 0, canonicalizing_jump
	canonicalizing_jump:

	# Set all segment registers to point to the very first segment (lowest 64k of memory),
	# where this stage stores all it's data.
	# That way, we can avoid dealing with far data.
	mov ax, 0x00
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	# Re-enable interrupts
	sti

	# Set SP
	mov sp, boot0_stack_top

	# Now ready to announce ourselves
	push si
	lea si, boot0_welcome_message
	call boot0_bios_print
	pop si

	# Reset floppy controller
	mov ah, 0x00
	int 0x13
	jc boot0_floppy_reset_error

	# Read boot1
	mov al, 10               # Ten sectors
	mov ch, 0                # Zeroth track
	mov cl, 2                # Second sector
	mov dh, 0                # Zeroth head
	# Do not touch dl, as it has the correct drive set by BIOS already
	mov bx, BOOT1_START_ADDR # Destination
	# Call
	mov ah, 0x02
	int 0x13
	# TODO: Should be retried 3 times on error
	jc boot0_floppy_read_error

	# Jump to boot1
	push si
	lea si, boot0_exit_message
	call boot0_bios_print
	pop si
	jmp BOOT1_START_ADDR


# Pad out to exactly 510 bytes, leaving last 2 in sector for signature
.fill 510-(.-_boot0_start), 1, 0

# Signature expected by boot firmware
.byte 0x55
.byte 0xAA
