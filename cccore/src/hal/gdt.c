#include "include/gdt.h"

#include <stddef.h>
#include <stdint.h>

static const uint64_t ONE = 1;  // Because of stupid integer promotion rules
static const uint64_t GDT_FLAG_RW = ONE << 41;
static const uint64_t GDT_FLAG_CODE = ONE << 43;
static const uint64_t GDT_FLAG_CODE_OR_DATA = ONE << 44;
static const uint64_t GDT_FLAG_PRESENT = ONE << 47;
static const uint64_t GDT_FLAG_LONG = ONE << 53;

/// Descriptor telling the CPU where to find the GDT.
struct __attribute__((__packed__)) gdt_info_t {
    uint16_t size;
    uint64_t offset;
};

// We only set up identity mappings and never touch them again, so this amount suffices
#define GDT_NUM_ENTRIES 2
static __attribute__((aligned(16))) uint64_t GDT[GDT_NUM_ENTRIES];
static __attribute__((aligned(16))) __attribute__((used)) struct gdt_info_t GDT_INFO = {
    .size = (GDT_NUM_ENTRIES * sizeof(uint64_t)) - 1,
    .offset = (uint64_t)GDT,
};

const uint16_t GDT_CODE_IDX = 8;

void gdt_init_flat(void) {
    // Null descriptor
    GDT[0] = 0x00;
    // 64-bit code (base and limit ignored by CPU)
    // Per AMD docs some of these are not needed in long mode, but at least bochs chokes on the descriptor if they're
    // left out.
    GDT[1] = GDT_FLAG_RW | GDT_FLAG_CODE | GDT_FLAG_CODE_OR_DATA | GDT_FLAG_PRESENT | GDT_FLAG_LONG;
    // 64-bit data descriptor not needed because we don't use compatibility mode

    __asm__ volatile(
        ".intel_syntax noprefix \n\t"
        "wbinvd                 \n\t"  // Flush CPU caches
        "lgdt [GDT_INFO]        \n\t"
        "mov ax, 0              \n\t"  // https://forum.osdev.org/viewtopic.php?f=1&t=31557&start=0
        "mov ss, ax             \n\t"
        "mov ds, ax             \n\t"
        "mov es, ax             \n\t"
        "mov fs, ax             \n\t"
        "mov gs, ax             \n\t"
        "lea rax, [rip + 0x05]  \n\t"  // Note that instructions have variable sizes
        "pushq 0x08             \n\t"
        "pushq rax              \n\t"
        "retfq                  \n\t"
        ".flush:                \n\t"
        ".att_syntax prefix     \n\t"
        :
        :
        : "rax");
}
