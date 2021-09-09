#include "../include/ccnonstd/io.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void vprintf_uint_to_base(const vprintf_sink sink, unsigned int uint, unsigned int base) {
    const char* digits = "0123456789ABCDEF";

    // 0 is a special case
    if (uint == 0) {
        sink('0');
        return;
    }

    // Would be large enough for even a hypothetical machine with 128-bit unsigned int
    const size_t buf_size = 64;
    char buf[buf_size];
    memset(buf, '\0', buf_size * sizeof(char));

    for (size_t i = 0; i < buf_size && uint != 0; i++) {
        const size_t digit = (size_t)(uint % base);
        buf[i] = digits[digit];
        uint /= base;
    }

    if (uint != 0) {
        const size_t digit = (size_t)uint;
        sink(digits[digit]);
    }

    // Order of digits is reversed
    size_t i = buf_size;
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
        if (c == '%') {
            // This works for single-character format specifiers
            i++;
            const char fmt = format[i];
            // Predeclared because vars can't be declared in switch
            char* cc;
            char ch;
            unsigned int uint;
            int sint;
            switch (fmt) {
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
                    uint = va_arg(vlist, unsigned int);
                    vprintf_uint_to_base(sink, uint, 10);
                    break;
                case 'x':
                    uint = va_arg(vlist, unsigned int);
                    vprintf_uint_to_base(sink, uint, 16);
                    break;
                case 'o':
                    uint = va_arg(vlist, unsigned int);
                    vprintf_uint_to_base(sink, uint, 8);
                    break;
                case 'd':
                case 'i':
                    sint = va_arg(vlist, int);
                    vprintf_sint_to_decimal(sink, sint);
                    break;
                case 'b':
                    // This one is non-standard. It prints an unsigned integer as binary.
                    uint = va_arg(vlist, unsigned int);
                    vprintf_uint_to_base(sink, uint, 2);
                    break;
                case '%':
                    sink('%');
                    break;
                default:
                    return -1;
                    break;
            }
            i++;
        } else {
            sink(c);
            i++;
        }
        c = format[i];
    }
    return 0;
}
