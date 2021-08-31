#include "../include/ccnonstd/io.h"

#include <stdarg.h>
#include <stddef.h>

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
