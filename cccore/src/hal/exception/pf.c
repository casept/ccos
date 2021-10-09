#include <stdint.h>

#include "../../common.h"
#include "../include/interrupt.h"

static const uint8_t INTERRUPT_NUM = 14;

static void pf(struct interrupt_isr_data_t *data) {
    // Determine which virtual address caused fault
    uint64_t cr2;
    __asm__ volatile(
        ".intel_syntax noprefix \n\t"
        "mov rax, cr2           \n\t"
        "mov %0, rax            \n\t"
        ".att_syntax prefix     \n\t"
        : "=r"(cr2)
        :
        : "rax");

    // Parse the error code
    const uint64_t err = data->int_arg;
    const uint64_t present = err & 0x0000000000000001;
    const uint64_t write = (err & 0x0000000000000002) >> 1;
    const uint64_t user = (err & 0x0000000000000004) >> 2;
    const uint64_t reserved_write = (err & 0x0000000000000008) >> 3;
    const uint64_t if_nx = (err & 0x0000000000000010) >> 4;
    kpanicf(
        "PF at virtual address 0x%x\npresent: %d, write: %d, user: %d\n reserved write: %d, instruction fetch (NX): "
        "%d\n",
        cr2, present, write, user, reserved_write, if_nx);
}

void exception_pf_register_default(void) { interrupt_register(pf, INTERRUPT_NUM); }
