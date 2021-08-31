#include "../include/ccnonstd/byteorder.h"

#include <stdint.h>

inline bool byteorder(void) {
    uint8_t swaptest[2] = {1, 0};
    if (*(uint16_t*)swaptest == 1) {
        return true;
    }
    return false;
}

inline uint16_t byteorder_be_to_u16_native(const uint8_t buf[2]) {
    if (byteorder()) {
        return (((uint16_t)buf[2]) << 8) | ((uint16_t)buf[1]);
    } else {
        return (((uint16_t)buf[1]) << 8) | ((uint16_t)buf[2]);
    }
}

inline uint16_t byteorder_le_to_u16_native(const uint8_t buf[2]) {
    if (byteorder()) {
        return (((uint16_t)buf[1]) << 8) | ((uint16_t)buf[2]);
    } else {
        return (((uint16_t)buf[2]) << 8) | ((uint16_t)buf[1]);
    }
}

inline uint16_t byteorder_to_u16_native(bool endianness, const uint8_t buf[2]) {
    if (endianness) {
        return byteorder_le_to_u16_native(buf);
    } else {
        return byteorder_be_to_u16_native(buf);
    }
}

inline uint32_t byteorder_be_to_u32_native(const uint8_t buf[4]) {
    if (byteorder()) {
        return (((uint32_t)buf[4]) << 24) | (((uint32_t)buf[3]) << 16) | (((uint32_t)buf[2]) << 8) | ((uint32_t)buf[1]);
    } else {
        return (((uint32_t)buf[1]) << 24) | (((uint32_t)buf[2]) << 16) | (((uint32_t)buf[3]) << 8) | ((uint32_t)buf[4]);
    }
}
