#include "../include/ccelf.h"

#include <ccnonstd/byteorder.h>
#include <ccnonstd/memory.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const uint8_t ELF_HDR_MAGIC[ELF_HDR_MAGIC_LEN] = {0x7F, 'E', 'L', 'F'};
static const uint32_t ELF_SUPPORTED_HDR_VERSION = 1;
static const uint8_t ELF_HDR_FLAGS_EMPTY[ELF_HDR_FLAGS_LEN] = {0, 0, 0, 0};
static elf_fatalf_handler_t ELF_FATALF_HANDLER = NULL;

void elf_register_fatalf(elf_fatalf_handler_t h) { ELF_FATALF_HANDLER = h; }

static void elf_fatalf(const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    ELF_FATALF_HANDLER(format, vlist);
    while (true) {
    }
    va_end(vlist);
}

/// Parse the ELF header.
///
/// This function checks whether the header is valid ELF64,
/// but not whether it's compatible with the bootloader.
static struct elf64_header_t elf64_header_parse(const uint8_t* data) {
    size_t i = 0;
    struct elf64_header_t hdr;
    memset(&hdr, 0, sizeof(hdr));

    memcpy(&hdr.magic, &data[i], ELF_HDR_MAGIC_LEN);
    i += ELF_HDR_FLAGS_LEN;
    if (!memcmp_bool(hdr.magic, ELF_HDR_MAGIC, ELF_HDR_MAGIC_LEN)) {
        elf_fatalf("ELF executable has invalid ELF header magic (expected {%c, %c, %c, %c}, got {%c, %c, %c, %c})",
                   ELF_HDR_MAGIC[0], ELF_HDR_MAGIC[1], ELF_HDR_MAGIC[2], ELF_HDR_MAGIC[3], hdr.magic[0], hdr.magic[1],
                   hdr.magic[2], hdr.magic[3]);
    }

    const uint8_t bitness = data[i];
    i += 1;
    switch (bitness) {
        case 1:
            hdr.is_64bit = false;
            elf_fatalf("ELF executable is not 64-bit");
            break;
        case 2:
            hdr.is_64bit = true;
            break;
        default:
            elf_fatalf("ELF executable has undefined bitness value");
            break;
    }

    const uint8_t endianness = data[i];
    i += 1;
    switch (endianness) {
        case 1:
            hdr.is_be = false;
            break;
        case 2:
            hdr.is_be = true;
            break;
        default:
            elf_fatalf("ELF executable has undefined endianness value");
            break;
    }

    hdr.hdr_version = data[i];
    i += 1;
    if (hdr.hdr_version != ELF_SUPPORTED_HDR_VERSION) {
        elf_fatalf("ELF executable ELF header version not supported");
    }

    hdr.abi = (enum elf_abi_t)data[i];
    i += 1;
    if (hdr.abi != ELF_ABI_SYSV) {
        elf_fatalf("ELF executable has undefined ABI value");
    }

    i += 8;  // Padding

    hdr.kind = (enum elf_kind_t)byteorder_to_u16_native(!hdr.is_be, &data[i]);
    i += 2;

    hdr.isa = (enum elf_isa_t)byteorder_to_u16_native(!hdr.is_be, &data[i]);
    i += 2;

    hdr.elf_version = byteorder_to_u32_native(!hdr.is_be, &data[i]);
    i += 4;
    return hdr;
}

/// Checks whether this ELF header describes an ELF executable we actually support.
/// That means that it's, for x86_64, for the System V ABI etc.
static void elf_header_check_compat(const struct elf64_header_t* const hdr) {
    if (hdr->kind != ELF_KIND_EXECUTABLE) {
        elf_fatalf("kernel executable declares not actually being an ELF executable");
    }

    if (hdr->isa != ELF_ISA_X86_64) {
        elf_fatalf("kernel executable ISA is not x86_64");
    }

    if (!memcmp_bool(hdr->flags, ELF_HDR_FLAGS_EMPTY, ELF_HDR_FLAGS_LEN)) {
        elf_fatalf("kernel executable ELF header contains unknown flags");
    }
}

struct elf64_header_t elf64_header_parse_and_validate(const uint8_t* data) {
    struct elf64_header_t hdr = elf64_header_parse(data);
    elf_header_check_compat(&hdr);
    return hdr;
}
