#include <ccelf.h>
#include <ccvga.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// Where the real mode code loaded our kernel's binary to (in low memory).
const uint8_t* BOOT1_KERNEL_COPY = (const uint8_t*)0x0000F000;

/// The entrypoint from ASM.
void boot1_cmain(void) {
    vga_clear();

    struct elf64_header_t hdr = elf64_header_parse_and_validate(BOOT1_KERNEL_COPY);
    (void)hdr;
    vga_printf("boot1: parsed and validated kernel ELF\n");

    while (true) {
    }
}
