#include "common.h"

#include <ccvga.h>
#include <stdarg.h>
#include <stdbool.h>

#include "hal/include/serial.h"

void kprint_init(void) {
    vga_clear();
    serial_com1_init();
}

void kprintf(const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    vga_vprintf(format, vlist);
    serial_com1_vprintf(format, vlist);
}

void __attribute__((noreturn)) kpanicf(const char* format, ...) {
    vga_printf("PANIC: ");
    va_list vlist;
    va_start(vlist, format);
    vga_vprintf(format, vlist);
    serial_com1_vprintf(format, vlist);
    __asm__ volatile("cli; hlt");
    // To make the stupid compiler happy
    while (true) {
    }
}
