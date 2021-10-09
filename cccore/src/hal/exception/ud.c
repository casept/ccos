#include <stdint.h>

#include "../../common.h"
#include "../include/interrupt.h"

static const uint8_t INTERRUPT_NUM = 6;

static void ud(struct interrupt_isr_data_t *data) {
    const uint64_t instr_addr = data->int_return;
    kpanicf("UD while executing address 0x%x", instr_addr);
}

void exception_ud_register_default(void) { interrupt_register(ud, INTERRUPT_NUM); }
