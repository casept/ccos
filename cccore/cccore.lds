/* -*- mode: linkerscript -*- */
/* vi: set ft=linkerscript : */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

/* Bootloader configures MMU based on these */
PHDRS {
	null       PT_NULL FLAGS(0) ;
	text       PT_LOAD FLAGS((1 << 0) | (1 << 2)) ; /* RX */
	rodata     PT_LOAD FLAGS((1 << 2)) ;            /* R */
	data       PT_LOAD FLAGS((1 << 1) | (1 << 2)) ; /* RW */
}

SECTIONS {
	/* Higher half, 2MB into physical memory */
	. = 0xffffffff80200000;

	.text : {
		*(.text*)
	} :text

	/* Move to the next page, because MMU permissions need to be different */
	. += 4096; /* 4K pages */
	/* Section containing bootloader metadata */
	.stivale2hdr : {
		KEEP(*(.stivale2hdr))
	} :rodata

	.rodata : {
		*(.rodata*)
	} :rodata

	/* Different permissions again */
	. += 4096;
	.data : {
		*(.data*)
	} :data

	.bss : {
		*(COMMON)
		*(.bss*)
	} :data
}