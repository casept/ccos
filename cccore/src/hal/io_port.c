#include "include/io_port.h"

#include <stdint.h>

uint8_t port_read_u8(uint16_t port) {
    uint8_t out;
    __asm__ volatile(
        ".intel_syntax noprefix \n\t"
        "mov dx, %[port]        \n\t"
        "in al, dx              \n\t"
        "mov %[out], al         \n\t"
        ".att_syntax prefix     \n\t"
        : [out] "=r"(out)
        : [port] "r"(port)
        : "eax", "edx");
    return out;
}

void port_write_u8(uint16_t port, uint8_t value) {
    __asm__ volatile(
        ".intel_syntax noprefix \n\t"
        "mov dx, %[port]        \n\t"
        "mov al, %[value]       \n\t"
        "out dx, al             \n\t"
        ".att_syntax prefix     \n\t"
        :
        : [port] "r"(port), [value] "r"(value)
        : "eax", "edx");
}
