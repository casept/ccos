#include "../include/ccnonstd/memory.h"

#include <stdbool.h>
#include <stddef.h>

bool memcmp_bool(const void* lhs, const void* rhs, size_t count) {
    const unsigned char* lhs_c = (const unsigned char*)lhs;
    const unsigned char* rhs_c = (const unsigned char*)rhs;

    for (size_t i = 0; i < count; i++) {
        if (lhs_c != rhs_c) {
            return false;
        }
        lhs_c++;
        rhs_c++;
    }
    return true;
}

volatile void* memset_volatile(volatile void* dest, int ch, size_t count) {
    volatile unsigned char* dest_uc = (volatile unsigned char*)dest;
    unsigned char ch_uc = (unsigned char)ch;

    for (size_t i = 0; i < count; i++) {
        dest_uc[i] = ch_uc;
    }

    return dest;
}

volatile void* memcpy_volatile(volatile void* dest, const volatile void* src, size_t count) {
    volatile unsigned char* dest_uc = (volatile unsigned char*)dest;
    const volatile unsigned char* src_uc = (const volatile unsigned char*)src;

    for (size_t i = 0; i < count; i++) {
        dest_uc[i] = src_uc[i];
    }

    return dest;
}
