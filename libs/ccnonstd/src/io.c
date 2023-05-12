#include "../include/ccnonstd/io.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void vprintf_uint_to_base(const vprintf_sink sink, unsigned int x, unsigned int base) {
    const char* digits = "0123456789ABCDEF";

    // 0 is a special case
    if (x == 0) {
        sink('0');
        return;
    }

    // Would be large enough for even a hypothetical machine with 128-bit unsigned int
    const size_t buf_size = 64;
    char buf[buf_size];
    memset(buf, '\0', buf_size * sizeof(char));

    for (size_t i = 0; i < buf_size && x != 0; i++) {
        const size_t digit = (size_t)(x % base);
        buf[i] = digits[digit];
        x /= base;
    }

    if (x != 0) {
        const size_t digit = (size_t)x;
        sink(digits[digit]);
    }

    // Order of digits is reversed
    size_t i = buf_size - 1;
    while (true) {
        if (buf[i] != '\0') {
            sink(buf[i]);
        }
        if (i == 0) {
            break;
        }
        i--;
    }
}

static void vprintf_uint_to_base_long(const vprintf_sink sink, unsigned long long x, unsigned int base) {
    const char* digits = "0123456789ABCDEF";

    // 0 is a special case
    if (x == 0) {
        sink('0');
        return;
    }

    // Would be large enough for even a hypothetical machine with 128-bit unsigned int
    const size_t buf_size = 64;
    char buf[buf_size];
    memset(buf, '\0', buf_size * sizeof(char));

    for (size_t i = 0; i < buf_size && x != 0; i++) {
        const size_t digit = (size_t)(x % base);
        buf[i] = digits[digit];
        x /= base;
    }

    if (x != 0) {
        const size_t digit = (size_t)x;
        sink(digits[digit]);
    }

    // Order of digits is reversed
    size_t i = buf_size - 1;
    while (true) {
        if (buf[i] != '\0') {
            sink(buf[i]);
        }
        if (i == 0) {
            break;
        }
        i--;
    }
}

static void vprintf_sint_to_decimal(const vprintf_sink sink, int sint) {
    if (sint < 0) {
        sink('-');
    }

    const unsigned int uint = (unsigned int)abs(sint);
    vprintf_uint_to_base(sink, uint, 10);
}

int vprintf_generic(const vprintf_sink sink, const char* format, va_list vlist) {
    size_t i = 0;
    char c = format[i];
    while (c != '\0') {
        // Process format specifiers
        bool is_long = false;
        if (c == '%') {
        // Look at the next format specifier character
        process_format_specifier:
            i++;
            const char fmt = format[i];
            // Pre declared because vars can't be declared in switch
            char* cc;
            char ch;
            unsigned int x;
            unsigned long long x_long;
            int x_signed;
            switch (fmt) {
                case 'l':
                    // Shift into "long" mode and evaluate the next format specifier character
                    is_long = true;
                    goto process_format_specifier;
                case 's':
                    cc = va_arg(vlist, char*);
                    while (*cc != '\0') {
                        sink(*cc);
                        cc++;
                    }
                    break;
                case 'c':
                    // Is promoted to an int and then immediately demoted back to char... facepalm
                    ch = va_arg(vlist, int);
                    sink(ch);
                    break;
                case 'u':
                    if (is_long) {
                        x_long = va_arg(vlist, unsigned long long);
                        vprintf_uint_to_base_long(sink, x_long, 10);
                        is_long = false;
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 10);
                    }
                    break;
                case 'x':
                    if (is_long) {
                        x_long = va_arg(vlist, unsigned long long);
                        vprintf_uint_to_base_long(sink, x_long, 16);
                        is_long = false;
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 16);
                    }
                    break;
                case 'o':
                    if (is_long) {
                        x_long = va_arg(vlist, unsigned long long);
                        vprintf_uint_to_base_long(sink, x_long, 8);
                        is_long = false;
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 8);
                    }
                    break;
                case 'd':
                case 'i':
                    // TODO: Handle long
                    x_signed = va_arg(vlist, int);
                    vprintf_sint_to_decimal(sink, x_signed);
                    break;
                case 'b':
                    if (is_long) {
                        x_long = va_arg(vlist, unsigned long long);
                        vprintf_uint_to_base_long(sink, x_long, 2);
                        is_long = false;
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 2);
                    }
                    break;
                case '%':
                    // Escaped literal %
                    sink('%');
                    break;
                default:
                    return -1;
                    break;
            }
            i++;
        } else {
            // This is a literal character
            sink(c);
            i++;
        }
        c = format[i];
    }
    return 0;
}
