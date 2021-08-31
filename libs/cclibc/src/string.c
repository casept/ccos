#include "../include/string.h"

#include <stddef.h>

void* memset(void* dest, int ch, size_t count) {
    unsigned char* dest_uc = (unsigned char*)dest;
    unsigned char ch_uc = (unsigned char)ch;

    for (size_t i = 0; i < count; i++) {
        dest_uc[i] = ch_uc;
    }

    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    unsigned char* dest_uc = (unsigned char*)dest;
    const unsigned char* src_uc = (const unsigned char*)src;

    for (size_t i = 0; i < count; i++) {
        dest_uc[i] = src_uc[i];
    }

    return dest;
}
