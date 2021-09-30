#include <stdbool.h>
#include <stddef.h>

#include "common.h"
#include "hal/include/exception.h"
#include "hal/include/gdt.h"
#include "hal/include/interrupt.h"
#include "hal/include/timer.h"
#include "stivale2.h"

#define STACK_SIZE 4096
static uint8_t stack[STACK_SIZE];

__attribute__((section(".stivale2hdr"), used)) static struct stivale2_header stivale_hdr = {
    .entry_point = 0,                           // Leave ELF default
    .stack = (uintptr_t)stack + sizeof(stack),  // Stack grows downwards
    .flags = (1 << 1) | (1 << 2),               // Higher half w/ MMU configured as in linker script
    .tags = 0,                                  // No tags means the bootloader will give us CGA text mode or bust
};

void kmain(void);

// NOLINTNEXTLINE (bugprone-reserved-identifier)
void _start(struct stivale2_struct *config) {
    (void)config;  // Don't care about stivale data for now

    // TODO: Zero out .bss
    kmain();
    while (true) {
    }
}

void test_timer(void) { kprintf("Tick\n"); }

void kmain(void) {
    interrupt_disable();
    kprint_init();
    kprintf("Hello\n");

    gdt_init_flat();
    interrupt_init();
    exception_register_default();

    timer_enable(100, test_timer);
    interrupt_enable();
}
