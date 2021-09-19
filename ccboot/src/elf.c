#include <ccelf.h>
#include <ccvga.h>
#include <stdint.h>

static const uint32_t ELF_HDR_FLAGS_EMPTY = 0;

/// Checks whether this ELF header describes an ELF executable we actually support.
/// That means that it's, for x86_64, for the System V ABI etc.
static void header_check_compat(const struct elf64_header_t* const hdr) {
    if (hdr->kind != ELF_KIND_EXECUTABLE) {
        vga_fatalf("kernel executable declares not actually being an ELF executable\n");
    }

    if (hdr->isa != ELF_ISA_X86_64) {
        vga_fatalf("kernel executable ISA is not x86_64\n");
    }

    if (hdr->flags != ELF_HDR_FLAGS_EMPTY) {
        vga_fatalf("kernel executable ELF header contains unknown flags\n");
    }
}

struct elf64_header_t elf64_header_parse_and_validate(const uint8_t* data) {
    struct elf64_header_t hdr = elf64_header_parse(data);
    header_check_compat(&hdr);
    return hdr;
}
