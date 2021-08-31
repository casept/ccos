#include "../include/ccvga.h"

#include <ccnonstd/io.h>
#include <ccnonstd/memory.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const size_t VGA_TEXTBUF_WIDTH = 80;
static const size_t VGA_TEXTBUF_HEIGHT = 25;
static size_t VGA_CURSOR_X = 0;
static size_t VGA_CURSOR_Y = 0;
// We assume a color output here, it's 2021 after all
static volatile uint16_t* VGA_TEXTBUF = (volatile uint16_t*)0x000B8000;

static const uint16_t VGA_WHITE_ON_BLACK_NOBLINK = 0x0F00;

void vga_clear(void) {
    VGA_CURSOR_X = 0;
    VGA_CURSOR_Y = 0;
    memset_volatile(VGA_TEXTBUF, 0x00, VGA_TEXTBUF_WIDTH * VGA_TEXTBUF_HEIGHT * sizeof(uint16_t));
}

static void vga_newline(void) {
    VGA_CURSOR_X = 0;
    VGA_CURSOR_Y++;
}

static void vga_putc(const char c) {
    const uint16_t vga_char = (uint16_t)c | VGA_WHITE_ON_BLACK_NOBLINK;

    switch (c) {
        case '\n':
            vga_newline();
            break;
        default:
            VGA_TEXTBUF[(VGA_TEXTBUF_WIDTH * VGA_CURSOR_Y) + VGA_CURSOR_X] = vga_char;
            VGA_CURSOR_X++;
            break;
    }

    if (VGA_CURSOR_X >= VGA_TEXTBUF_WIDTH) {
        vga_newline();
    }

    if (VGA_CURSOR_Y >= VGA_TEXTBUF_HEIGHT) {
        VGA_CURSOR_Y = 0;
    }
    // TODO: Scrolling etc. (if even needed, bootloader shouldn't produce that much output)
}

void vga_vprintf(const char* format, va_list vlist) {
    if (vprintf_generic(vga_putc, format, vlist) == -1) {
        vga_fatalf("vga_printf(): Failed");
    }
}

void vga_printf(const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    vga_vprintf(format, vlist);
    va_end(vlist);
}

void vga_vfatalf(const char* format, va_list vlist) {
    vga_printf("\nboot1: FATAL: ");
    vga_vprintf(format, vlist);
    vga_putc('\n');
    while (true) {
    }
}

void vga_fatalf(const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    vga_printf("\nboot1: FATAL: ");
    vga_vprintf(format, vlist);
    vga_putc('\n');
    va_end(vlist);
    while (true) {
    }
}
