#include "include/timer.h"

#include <stddef.h>
#include <stdint.h>

#include "../common.h"
#include "../interrupt/controller/pic.h"
#include "include/interrupt.h"
#include "include/io_port.h"

static const uint8_t PIT_IRQ = 0;
static const uint32_t PIT_RATE_HZ = 1193182;

static const uint16_t PIT_PORT_CHANNEL_0 = 0x40;
static const uint16_t PIT_PORT_COMMAND = 0x43;

static const uint8_t PIT_CHANNEL_0 = 0b00000000;
static const uint8_t PIT_ACCESS_LOHI = 0b00110000;
static const uint8_t PIT_MODE_2_RATE = 0b00000100;
static const uint8_t PIT_BINARY = 0b00000000;

timer_callback_t PIT_CALLBACK = NULL;

/// Called by the ASM stub and performs the parts of IRQ handling that can be done in C.
void pit_isr(void) {
    interrupt_ack(pic_idt_slot_to_irq(PIT_IRQ));
    PIT_CALLBACK();
}

/// Programs the legacy PIT timer.
/// Timer causes IRQ 0 interrupts.
static void pit_enable(uint32_t tickrate_hz, timer_callback_t callback) {
    interrupt_disable();  // TODO: Not sufficient w/ multiprocessor
    // Convert frequency to timer reload value
    const uint64_t reload_value_unclamped = PIT_RATE_HZ / tickrate_hz;
    uint16_t reload_value = (uint16_t)reload_value_unclamped;
    // If value overflows, clamp to max
    if (reload_value_unclamped > UINT16_MAX) {
        reload_value = UINT16_MAX;
    }

    const uint8_t settings = PIT_CHANNEL_0 | PIT_ACCESS_LOHI | PIT_MODE_2_RATE | PIT_BINARY;
    port_write_u8(PIT_PORT_COMMAND, settings);

    const uint8_t lo_byte = (uint8_t)(reload_value & 0x00FF);
    port_write_u8(PIT_PORT_CHANNEL_0, lo_byte);
    const uint8_t hi_byte = (uint8_t)((reload_value & 0xFF00) >> 8);
    port_write_u8(PIT_PORT_CHANNEL_0, hi_byte);

    PIT_CALLBACK = callback;
    // TODO: Build better abstraction to decouple from PIC
    interrupt_register(pit_isr, pic_irq_to_idt_slot(PIT_IRQ));

    interrupt_enable();
    kprintf("PIC IRR: %b\n", pic_get_irr());
    kprintf("PIC ISR: %b\n", pic_get_isr());
}

void timer_enable(uint32_t tickrate_hz, timer_callback_t callback) {
    // For now, only PIT is supported
    pit_enable(tickrate_hz, callback);
}
