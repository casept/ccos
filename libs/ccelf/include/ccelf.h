#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

/// Length of ELF header magic number
#define ELF_HDR_MAGIC_LEN 4
/// Length of ELF version
#define ELF_VERSION_LEN 4

#define ELF_PROGRAM_SEGMENT_FLAG_EXECUTABLE 0x01
#define ELF_PROGRAM_SEGMENT_FLAG_WRITABLE 0x02
#define ELF_PROGRAM_SEGMENT_FLAG_READABLE 0x04

/// Signature for the error handling function
typedef void (*elf_fatalf_handler_t)(const char* format, va_list args);

void elf_register_fatalf(elf_fatalf_handler_t h);

// Incomplete, but these are the ones we care about
enum elf_abi_t {
    ELF_ABI_SYSV = 0x00,
};

enum elf_kind_t {
    ELF_KIND_RELOCATABLE = 0x01,
    ELF_KIND_EXECUTABLE = 0x02,
    ELF_KIND_SHARED = 0x03,
    ELF_KIND_CORE = 0x04,
};

enum elf_isa_t {
    ELF_ISA_X86 = 0x03,
    ELF_ISA_X86_64 = 0x3E,
};

/// Header for a 64-bit ELF.
struct elf64_header_t {
    uint8_t magic[ELF_HDR_MAGIC_LEN];
    bool is_64bit;
    bool is_be;
    uint8_t hdr_version;
    enum elf_abi_t abi;
    enum elf_kind_t kind;
    enum elf_isa_t isa;
    uint32_t elf_version;
    uint64_t entrypoint_position;
    uint64_t program_hdr_table_position;
    uint64_t section_hdr_table_position;
    uint32_t flags;
    uint16_t hdr_size;
    uint16_t program_hdr_table_entry_size;
    uint16_t num_program_hdr_table_entries;
    uint16_t section_hdr_table_entry_size;
    uint16_t num_section_hdr_table_entries;
    uint16_t section_hdr_table_names_idx;
};

/// Types of program segments.
enum elf_program_segment_kind_t {
    ELF_PROGRAM_SEGMENT_KIND_NULL = 0x00,
    ELF_PROGRAM_SEGMENT_KIND_LOAD = 0x01,
    ELF_PROGRAM_SEGMENT_KIND_DYNAMIC = 0x02,
    ELF_PROGRAM_SEGMENT_KIND_INTERP = 0x03,
    ELF_PROGRAM_SEGMENT_KIND_NOTE = 0x04,
};

/// Program header for a 64-bit ELF.
struct elf64_program_header_t {
    enum elf_program_segment_kind_t kind;
    uint32_t flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t alignment;
};

/// Section header for a 64-bit ELF.
struct elf64_section_header_t {
    uint64_t name;
    uint64_t type;
    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint64_t link;
    uint64_t info;
    uint64_t addralign;
    uint64_t entsize;
};

/// Parses a given ELF header.
///
/// Does not bounds check the input data buffer.
struct elf64_header_t elf64_header_parse(const uint8_t* data);
