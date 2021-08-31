.intel_syntax noprefix
.code16
.section .boot1.text,"ax"

.global boot1_ensure_a20_active
boot1_ensure_a20_active:
	cli

	push ax
	call check_a20
	cmp ax, 1
	je is_active
	call enable_a20
	jmp is_active

	is_active:
		pop ax
		ret

# Cribbed this from https://wiki.osdev.org/A20_Line (with some adjustments for GAS),
# because writing all these legacy checks sounds like a massive pain.

# Function: check_a20
#
# Purpose: to check the status of the a20 line in a completely self-contained state-preserving way.
#		   The function can be modified as necessary by removing push's at the beginning and their
#		   respective pop's at the end if complete self-containment is not required.
#
# Returns: 0 in ax if the a20 line is disabled (memory wraps around)
#		   1 in ax if the a20 line is enabled (memory does not wrap around)
check_a20:
	pushf
	push ds
	push es
	push di
	push si

	cli

	xor ax, ax ; ax = 0
	mov es, ax

	not ax ; ax = 0xFFFF
	mov ds, ax

	mov di, 0x0500
	mov si, 0x0510

	mov al, BYTE PTR es:[di]
	push ax

	mov al, BYTE PTR ds:[si]
	push ax

	mov BYTE PTR es:[di], 0x00
	mov BYTE PTR ds:[si], 0xFF

	cmp BYTE PTR es:[di], 0xFF

	pop ax
	mov BYTE PTR ds:[si], al

	pop ax
	mov BYTE PTR es:[di], al

	mov ax, 0
	je check_a20__exit

	mov ax, 1

	check_a20__exit:
		pop si
		pop di
		pop es
		pop ds
		popf

		ret

enable_a20:
	cli

	push ax

	call	a20wait
	mov		al,0xAD
	out		0x64,al

	call	a20wait
	mov		al,0xD0
	out		0x64,al

	call	a20wait2
	in		al,0x60
	push	eax

	call	a20wait
	mov		al,0xD1
	out		0x64,al

	call	a20wait
	pop		eax
	or		al,2
	out		0x60,al

	call	a20wait
	mov		al,0xAE
	out		0x64,al


	call	a20wait
	sti

	pop ax
	ret

	a20wait:
		in		al,0x64
		test	al,2
		jnz		a20wait
		ret


	a20wait2:
		in		al,0x64
		test	al,1
		jz		a20wait2
		ret
