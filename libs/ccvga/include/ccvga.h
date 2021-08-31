#pragma once

#include <stdarg.h>

/// Clears the VGA display and resets cursor position.
void vga_clear(void);

/// `vprintf_generic` specialized for VGA text mode.
void vga_printf(const char* format, ...);
void vga_vprintf(const char* format, va_list vlist);

/// Prints message and halts execution.
void vga_fatalf(const char* format, ...);
void vga_vfatalf(const char* format, va_list vlist);
