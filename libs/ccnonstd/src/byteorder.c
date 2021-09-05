#include "../include/ccnonstd/byteorder.h"

#include <stddef.h>
#include <stdint.h>

bool byteorder(void) {
    // Alignment required for cast to be always legal
    uint8_t __attribute__((aligned(2))) swaptest[2] = {1, 0};
    if (*(uint16_t*)swaptest == 1) {
        return true;
    }
    return false;
}

uint16_t byteorder_be_to_u16_native(const uint8_t buf[2]) {
    uint16_t x;
    x = ((uint16_t)buf[0]) << 8;
    x |= ((uint16_t)buf[1]);
    return x;
}

uint16_t byteorder_le_to_u16_native(const uint8_t buf[2]) {
    uint16_t x;
    x = ((uint16_t)buf[1]) << 8;
    x |= ((uint16_t)buf[0]);
    return x;
}

uint16_t byteorder_to_u16_native(bool endianness, const uint8_t buf[2]) {
    if (endianness) {
        return byteorder_le_to_u16_native(buf);
    } else {
        return byteorder_be_to_u16_native(buf);
    }
}

uint32_t byteorder_be_to_u32_native(const uint8_t buf[4]) {
    uint32_t x;
    x = ((uint32_t)buf[0]) << 24;
    x |= ((uint32_t)buf[1]) << 16;
    x |= ((uint32_t)buf[2]) << 8;
    x |= ((uint32_t)buf[3]);
    return x;
}

uint32_t byteorder_le_to_u32_native(const uint8_t buf[4]) {
    uint32_t x;
    x = ((uint32_t)buf[3]) << 24;
    x |= ((uint32_t)buf[2]) << 16;
    x |= ((uint32_t)buf[1]) << 8;
    x |= ((uint32_t)buf[0]);
    return x;
}

uint32_t byteorder_to_u32_native(bool endianness, const uint8_t buf[4]) {
    if (endianness) {
        return byteorder_le_to_u32_native(buf);
    } else {
        return byteorder_be_to_u32_native(buf);
    }
}

uint64_t byteorder_be_to_u64_native(const uint8_t buf[8]) {
    uint64_t x;
    x = ((uint64_t)buf[0]) << 56;
    x |= ((uint64_t)buf[1]) << 48;
    x |= ((uint64_t)buf[2]) << 40;
    x |= ((uint64_t)buf[3]) << 32;
    x |= ((uint64_t)buf[4]) << 24;
    x |= ((uint64_t)buf[5]) << 16;
    x |= ((uint64_t)buf[6]) << 8;
    x |= ((uint64_t)buf[7]);
    return x;
}

uint64_t byteorder_le_to_u64_native(const uint8_t buf[8]) {
    uint64_t x;
    x = ((uint64_t)buf[7]) << 56;
    x |= ((uint64_t)buf[6]) << 48;
    x |= ((uint64_t)buf[5]) << 40;
    x |= ((uint64_t)buf[4]) << 32;
    x |= ((uint64_t)buf[3]) << 24;
    x |= ((uint64_t)buf[2]) << 16;
    x |= ((uint64_t)buf[1]) << 8;
    x |= ((uint64_t)buf[0]);
    return x;
}

uint64_t byteorder_to_u64_native(bool endianness, const uint8_t buf[8]) {
    if (endianness) {
        return byteorder_le_to_u64_native(buf);
    } else {
        return byteorder_be_to_u64_native(buf);
    }
}
