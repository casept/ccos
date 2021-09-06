#include <ccelf.h>
#include <ccvga.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "floppy.h"

extern struct elf64_header_t elf64_header_parse_and_validate(const uint8_t* data);

/// Where the real mode code loaded our kernel's binary to (in low memory).
uint8_t* BOOT1_KERNEL_COPY = (uint8_t*)0x0000F000;

/// The entrypoint from ASM.
void boot1_cmain(void) {
    vga_clear();

    vga_printf("boot1: Reading kernel from floppy\n");
    // First 18 sectors (first track) are reserved for bootloader,
    // rest is kernel.
    // For testing, reading the second track only is sufficient.
    // TODO: Rewrite to read entire floppy
    floppy_read(18, 18, BOOT1_KERNEL_COPY);
    vga_printf("boot1: Kernel has been read\n");

    vga_printf("boot1: Parsing and validating kernel ELF\n");
    elf_register_fatalf(vga_vprintf);
    struct elf64_header_t hdr = elf64_header_parse_and_validate(BOOT1_KERNEL_COPY);
    (void)hdr;
    vga_printf("boot1: parsed and validated kernel ELF\n");

    while (true) {
    }
}
