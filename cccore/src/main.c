#include <stdbool.h>
#include <stddef.h>

#include "stivale2.h"

#define STACK_SIZE 4096
static uint8_t stack[STACK_SIZE];

__attribute__((section(".stivale2hdr"), used)) static struct stivale2_header stivale_hdr = {
    .entry_point = 0,                           // Leave ELF default
    .stack = (uintptr_t)stack + sizeof(stack),  // Stack grows downwards
    .flags = (1 << 1) | (1 << 2),               // Higher half w/ MMU configured as in linker script
    .tags = 0,                                  // No tags means the bootloader will give us CGA text mode or bust
};

void _start(struct stivale2_struct *config) {
    (void)config;  // Don't care about stivale data for now
    while (true) {
    }
}
