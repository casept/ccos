#include "include/gdt.h"

#include <stddef.h>
#include <stdint.h>

/// Global descriptor table entry telling the MMU about a mapping.
typedef uint64_t gdt_entry_t;

// Relevant flags for long mode
static const gdt_entry_t ONE =
    1;  // Define this because using literal ones would cause C to interpret it as an int due to retarded type deduction
static const gdt_entry_t GDT_FLAG_CONFORMING = ONE << 32;
static const gdt_entry_t GDT_FLAG_PRESENT = ONE << 15;
static const gdt_entry_t GDT_FLAG_LONG = ONE << 21;
static const gdt_entry_t GDT_FLAG_DEFAULT_OPERAND_SIZE = ONE << 22;

/// Descriptor telling the CPU where to find the GDT.
struct __attribute__((__packed__)) gdt_info_t {
    uint16_t size;
    uint64_t offset;
};

// We only set up identity mappings and never touch them again, so this amount suffices
#define GDT_NUM_ENTRIES 3
static __attribute__((aligned(16))) gdt_entry_t GDT[GDT_NUM_ENTRIES];
static __attribute__((aligned(16))) __attribute__((used)) struct gdt_info_t GDT_INFO = {
    .size = (GDT_NUM_ENTRIES * sizeof(gdt_entry_t)) - 1,
    .offset = (uint64_t)GDT,
};

const uint16_t GDT_CODE_IDX = 8;
const uint16_t GDT_DATA_IDX = 16;

void gdt_init_flat(void) {
    // Null descriptor
    GDT[0] = (gdt_entry_t)0x00;
    // 64-bit code (base and limit not checked by CPU)
    GDT[1] = GDT_FLAG_CONFORMING | GDT_FLAG_LONG | GDT_FLAG_PRESENT;
    // 64-bit data (base and limit not checked by CPU)
    GDT[2] = GDT_FLAG_PRESENT | GDT_FLAG_DEFAULT_OPERAND_SIZE;

    __asm__ volatile(
        ".intel_syntax noprefix  \n\t"
        "lgdt [GDT_INFO]         \n\t"
        ".att_syntax prefix      \n\t");
}
