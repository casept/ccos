#pragma once
#include <stdint.h>

/// Index of 64-bit code descriptor.
extern const uint16_t GDT_CODE_IDX;

/// Initialize a flat memory mapping.
void gdt_init_flat(void);
