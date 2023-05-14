#include <stddef.h>
#include <stdint.h>

#include "../../common.h"
#include "../include/interrupt.h"

static const uint8_t INTERRUPT_NUM = 13;

static void gpf(struct interrupt_isr_data_t *data) {
    const uint64_t instr_addr = data->rip;
    const uint64_t selector = data->int_arg;

    if (selector != 0) {
        // Decode the selector
        const uint8_t table_id = (selector >> 2) & 0x1;
        const uint16_t index = (selector >> 3) & 0x1FFF;
        const uint8_t external = (selector >> 15) & 0x1;
        const char *table = NULL;
        switch (table_id) {
            case 0:
                table = "GDT";
                break;
            case 1:
            case 3:
                table = "IDT";
                break;
            case 2:
                table = "LDT";
                break;
            default:
                kpanicf("%s: invalid table id: %u\n", __func__, table_id);
        }
        kpanicf("==== GPF ====\naddress: %p\nindex: %u\nexternal: %w\ntable: %s\n==============\n", instr_addr, index,
                external, table);
    } else {
        kpanicf("==== GPF ====\naddress: %p\n==============\n", instr_addr);
    }
}

void exception_gpf_register_default(void) { interrupt_register(gpf, INTERRUPT_NUM); }
