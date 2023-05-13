#include "../include/ccnonstd/io.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void vprintf_uint_to_base(const vprintf_sink sink, unsigned int x, unsigned int base, int padding) {
    const char* digits = "0123456789ABCDEF";

    // 0 is a special case
    if (x == 0) {
        sink('0');
        return;
    }

    // Would be large enough for even a hypothetical machine with 128-bit unsigned int
    const size_t buf_size = 64;
    char buf[buf_size];
    size_t num_digits = 0;
    memset(buf, '\0', buf_size * sizeof(char));

    for (size_t i = 0; i < buf_size && x != 0; i++) {
        num_digits++;
        const size_t digit = (size_t)(x % base);
        buf[i] = digits[digit];
        x /= base;
    }

    if (x != 0) {
        digits++;
        const size_t digit = (size_t)x;
        sink(digits[digit]);
    }

    // Add padding digits if necessary
    if (padding != -1 && num_digits < (size_t)padding) {
        for (size_t i = 0; i < (size_t)padding - num_digits; i++) {
            sink('0');
        }
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

static void vprintf_uint_to_base_long(const vprintf_sink sink, unsigned long long x, unsigned int base, int padding) {
    const char* digits = "0123456789ABCDEF";

    // 0 is a special case
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
        const size_t digit = (size_t)(x % base);
        buf[i] = digits[digit];
        x /= base;
    }

    if (x != 0) {
        num_digits++;
        const size_t digit = (size_t)x;
        sink(digits[digit]);
    }

    // Add padding digits if necessary
    if (padding != -1 && num_digits < (size_t)padding) {
        for (size_t i = 0; i < (size_t)padding - num_digits; i++) {
            sink('0');
        }
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
            char* cc;
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
                        vprintf_uint_to_base_long(sink, x_long, 10, padding);
                        is_long = false;
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 10, padding);
                    }
                    break;
                case 'x':
                    if (is_long) {
                        x_long = va_arg(vlist, unsigned long long);
                        vprintf_uint_to_base_long(sink, x_long, 16, padding);
                        is_long = false;
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 16, padding);
                    }
                    break;
                case 'o':
                    if (is_long) {
                        x_long = va_arg(vlist, unsigned long long);
                        vprintf_uint_to_base_long(sink, x_long, 8, padding);
                        is_long = false;
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
                        is_long = false;
                    } else {
                        x = va_arg(vlist, unsigned int);
                        vprintf_uint_to_base(sink, x, 2, padding);
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
            // This is a literal character, and no format specifier is expected anymore
            sink(c);
            i++;
        }
        c = format[i];
    }
    return 0;
}
