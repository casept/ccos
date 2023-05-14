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
    va_list vlist_serial;
    va_start(vlist_serial, format);
    va_list vlist_vga;
    va_copy(vlist_vga, vlist_serial);
    serial_com1_vprintf(format, vlist_serial);
    vga_vprintf(format, vlist_vga);
    va_end(vlist_vga);
    va_end(vlist_serial);
}

void __attribute__((noreturn)) kpanicf(const char* format, ...) {
    vga_printf("PANIC: ");
    serial_com1_printf("PANIC: ");
    va_list vlist_serial;
    va_start(vlist_serial, format);
    va_list vlist_vga;
    va_copy(vlist_vga, vlist_serial);
    serial_com1_vprintf(format, vlist_serial);
    vga_vprintf(format, vlist_serial);
    __asm__ volatile("cli; hlt");
    // To make the stupid compiler happy
    va_end(vlist_vga);
    va_end(vlist_serial);
    while (true) {
    }
}
