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

struct elf64_header_t elf64_header_parse(const uint8_t* data) {
    size_t i = 0;
    struct elf64_header_t hdr;
    memset(&hdr, 0, sizeof(hdr));

    memcpy(&hdr.magic, &data[i], ELF_HDR_MAGIC_LEN);
    i += ELF_HDR_MAGIC_LEN;
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

    hdr.entrypoint_position = byteorder_to_u64_native(!hdr.is_be, &data[i]);
    i += 8;

    hdr.program_hdr_table_position = byteorder_to_u64_native(!hdr.is_be, &data[i]);
    i += 8;

    hdr.section_hdr_table_position = byteorder_to_u64_native(!hdr.is_be, &data[i]);
    i += 8;

    hdr.flags = byteorder_to_u32_native(!hdr.is_be, &data[i]);
    i += 4;

    hdr.hdr_size = byteorder_to_u16_native(!hdr.is_be, &data[i]);
    i += 2;

    hdr.program_hdr_table_entry_size = byteorder_to_u16_native(!hdr.is_be, &data[i]);
    i += 2;

    hdr.num_program_hdr_table_entries = byteorder_to_u16_native(!hdr.is_be, &data[i]);
    i += 2;

    hdr.section_hdr_table_entry_size = byteorder_to_u16_native(!hdr.is_be, &data[i]);
    i += 2;

    hdr.num_section_hdr_table_entries = byteorder_to_u16_native(!hdr.is_be, &data[i]);
    i += 2;

    hdr.section_hdr_table_names_idx = byteorder_to_u16_native(!hdr.is_be, &data[i]);

    return hdr;
}
