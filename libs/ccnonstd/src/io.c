#include "../include/ccnonstd/io.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void vprintf_uint_to_base(const vprintf_sink sink, unsigned int x, unsigned int base, int padding) {
    const char* digits = "0123456789ABCDEF";

    // 0 is a special case, as it would otherwise be printed as an empty string
    if (x == 0) {
        sink('0');
        return;
    }

    // Would be large enough for even a hypothetical machine with 128-bit unsigned int
    const size_t buf_size = 64;
    char buf[buf_size];
    size_t num_digits = 0;
    memset(buf, '\0', buf_size * sizeof(char));

    for (size_t i = 0; i < buf_size; i++) {
        num_digits++;
        // The last digit is special because we don't want to print leading zeros
        if (x < base) {
            buf[i] = digits[x];
            break;
        }
        const size_t digit = (size_t)(x % base);
        buf[i] = digits[digit];
        x /= base;
    }

    // Add padding digits if necessary
    if (padding != -1 && num_digits < (size_t)padding) {
        size_t needed_padding = (size_t)padding - num_digits;
        for (size_t i = 0; i < needed_padding; i++) {
            sink('0');
        }
    }

    // Order of digits is reversed
    for (size_t i = buf_size - 1; true; i--) {
        if (buf[i] == '\0') continue;
        sink(buf[i]);
        if (i == 0) break;
    }
}

static void vprintf_uint_to_base_long(const vprintf_sink sink, unsigned long long x, unsigned int base, int padding) {
    const char* digits = "0123456789ABCDEF";

    // 0 is a special case, as it would otherwise be printed as an empty string
    if (x == 0) {
        sink('0');
        return;
    }

    // Would be large enough for even a hypothetical machine with 128-bit unsigned int
    const size_t buf_size = 64;
    char buf[buf_size];
    size_t num_digits = 0;
    memset(buf, '\0', buf_size * sizeof(char));

    for (size_t i = 0; i < buf_size; i++) {
        num_digits++;
        // The last digit is special because we don't want to print leading zeros
        if (x < base) {
            buf[i] = digits[x];
            break;
        }

        const size_t digit = (size_t)(x % base);
        buf[i] = digits[digit];
        x /= base;
    }

    // Add padding digits if necessary
    if (padding != -1 && num_digits < (size_t)padding) {
        size_t needed_padding = (size_t)padding - num_digits;
        for (size_t i = 0; i < needed_padding; i++) {
            sink('0');
        }
    }

    // Order of digits is reversed
    for (size_t i = buf_size - 1; true; i--) {
        if (buf[i] == '\0') continue;
        sink(buf[i]);
        if (i == 0) break;
    }
}

static void vprintf_sint_to_decimal(const vprintf_sink sink, int sint, int padding) {
    if (sint < 0) {
        sink('-');
    }

    const unsigned int uint = (unsigned int)abs(sint);
    vprintf_uint_to_base(sink, uint, 10, padding);
}

int vprintf_generic(const vprintf_sink sink, const char* format, va_list vlist) {
    size_t i = 0;
    char c = format[i];
    while (c != '\0') {
        // State we have to remember between format specifier characters
        bool is_long = false;
        int padding = -1;  // -1 => no padding

        if (c == '%') {
        // Look at the next format specifier character
        process_specifier_char:
            i++;
            const char fmt = format[i];
            // Pre declared because vars can't be declared in switch
            const char* str;
            char ch;
            unsigned int x;
            unsigned long long x_long;
            int x_signed;

            // Handle padding
            // TODO: Force padding to be directly after % and before format specifier
            if (fmt >= '0' && fmt <= '9') {
                // Remove sentinel value, to indicate padding is desired
                if (padding == -1) {
                    padding = 0;
                }

                int digit = fmt - '0';
                // TODO: Handle > 3 digits w/ a smarter approach
                if (padding < 10) {
                    padding = digit;
                } else if (padding < 100) {
                    // Shift the previous digit to the tens place and add the new digit to the ones place
                    padding = (padding * 10) + digit;
                } else if (padding < 1000) {
                    // Shift the previous digits to the hundreds and tens places and add the new digit to the ones place
                    padding = (padding * 100) + digit;
                }

                goto process_specifier_char;
            }

            // Handle non-padding format specifiers
            switch (fmt) {
                case 'l':
                    // Shift into "long" mode and evaluate the next format specifier character
                    is_long = true;
                    goto process_specifier_char;
                case 's':
                    str = va_arg(vlist, const char*);
                    while (*str != '\0') {
                        sink(*str);
                        str++;
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
                        vprintf_uint_to_base_long(sink, x_long, 10, padding);
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 10, padding);
                    }
                    break;
                case 'x':
                    if (is_long) {
                        x_long = va_arg(vlist, unsigned long long);
                        vprintf_uint_to_base_long(sink, x_long, 16, padding);
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 16, padding);
                    }
                    break;
                case 'o':
                    if (is_long) {
                        x_long = va_arg(vlist, unsigned long long);
                        vprintf_uint_to_base_long(sink, x_long, 8, padding);
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 8, padding);
                    }
                    break;
                case 'd':
                case 'i':
                    // TODO: Handle long
                    x_signed = va_arg(vlist, int);
                    vprintf_sint_to_decimal(sink, x_signed, padding);
                    break;
                case 'b':
                    if (is_long) {
                        x_long = va_arg(vlist, unsigned long long);
                        vprintf_uint_to_base_long(sink, x_long, 2, padding);
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 2, padding);
                    }
                    break;
                case 'p':
                    sink('0');
                    sink('x');
                    // Pointers are assumed to be 64-bit
                    x_long = va_arg(vlist, unsigned long long);
                    vprintf_uint_to_base_long(sink, x_long, 16, 16);
                    break;
                // Non-standard format specifier %w: Print an unsigned integer as "true" (>=1) or "false" (=0)
                case 'w':
                    x = va_arg(vlist, unsigned int);
                    if (x) {
                        sink('t');
                        sink('r');
                        sink('u');
                        sink('e');
                    } else {
                        sink('f');
                        sink('a');
                        sink('l');
                        sink('s');
                        sink('e');
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
            // This is a literal character, this particular format specifier is over
            sink(c);
            i++;
        }
        c = format[i];
    }
    return 0;
}
