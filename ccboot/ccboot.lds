/* -*- mode: linkerscript -*- */
/* vi: set ft=linkerscript : */

SECTIONS {
	. = 0x7C00;
	.boot0.text : { *(.boot0.text) }
	. = 0x1000;
	.boot1.text : {
		*(.boot1.text)
		/* Because of the C code */
		*(.text)
		*(.data)
		*(.rodata)
	}
}
