#include <stdint.h>

#include "../../common.h"
#include "../include/interrupt.h"

static const uint8_t INTERRUPT_NUM = 13;

static void gpf(struct interrupt_isr_data_t *data) {
    const uint64_t instr_addr = data->rip;
    const uint64_t selector = data->int_arg;

    if (selector != 0) {
        kpanicf("GPF while executing address %x, related to selector %u", instr_addr, selector);
    } else {
        kpanicf("GPF while executing address %x", instr_addr);
    }
}

void exception_gpf_register_default(void) { interrupt_register(gpf, INTERRUPT_NUM); }
