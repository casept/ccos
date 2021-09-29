#include "pic.h"

#include <stdbool.h>
#include <stdint.h>

#include "include/io_port.h"

// TODO: Remove
#include "../../common.h"

// Where the PIC chips are remapped to.
// Intel reserves vectors up to 0x1F for CPU-internal exceptions, so start right above that range.
static const uint8_t PIC1_VECTOR_BASE = 0x20;
static const uint8_t PIC2_VECTOR_BASE = 0x28;

// I/O ports used to program legacy PIC.
static const uint16_t PIC1_PORT_CMD = 0x20;
static const uint16_t PIC1_PORT_DATA = 0x21;
static const uint16_t PIC2_PORT_CMD = 0xA0;
static const uint16_t PIC2_PORT_DATA = 0xA1;

/// Values for first initialization word.
static const uint8_t PIC_ICW1_ICW4_NEEDED = 0b00000001;
static const uint8_t PIC_ICW1_MODE_CASCADE = 0b00000000;
static const uint8_t PIC_ICW1_INIT = 0b00010000;

/// Values for fourth initialization word.
static const uint8_t PIC_ICW4_8086 = 0b00000001;

/// PIC1 IRQ pin on which PIC2 is connected to PIC1
static const uint8_t PIC1_PIC2_IRQ = 0b00000100;
/// PIC2 cascade identity
static const uint8_t PIC2_CASCADE_ID = 0b00000010;

/// Commands that may be issued to the PIC.
enum pic_cmd_t {
    /// Configuration start command.
    /// 3 init words (bytes) expected after sending this.
    PIC_CMD_CONFIGURE = 0x11,
    /// End-of-interrupt command.
    /// Has to be sent to master (PIC1) for IRQ 0-7, and to both for others.
    PIC_CMD_EOI = 0x20,
    /// Disable the PIC.
    PIC_CMD_DISABLE = 0xFF,
    /// Read the IRQ ready register.
    PIC_CMD_READ_IRR = 0x0A,
    /// Read the in-service register.
    PIC_CMD_READ_ISR = 0x0B,
};

uint8_t pic_irq_to_idt_slot(uint8_t irq) { return PIC1_VECTOR_BASE + irq; }

uint8_t pic_idt_slot_to_irq(uint8_t idt_slot) { return idt_slot - PIC1_VECTOR_BASE; }

void pic_enable(void) {
    // TODO: Real HW might require wait after each I/O

    // Save mask stating which IRQs are active
    const uint8_t mask_1 = port_read_u8(PIC1_PORT_DATA);
    const uint8_t mask_2 = port_read_u8(PIC2_PORT_DATA);

    // Start cascade init
    port_write_u8(PIC1_PORT_CMD, PIC_ICW1_INIT | PIC_ICW1_ICW4_NEEDED | PIC_ICW1_MODE_CASCADE);
    port_write_u8(PIC1_PORT_CMD, PIC_ICW1_INIT | PIC_ICW1_ICW4_NEEDED | PIC_ICW1_MODE_CASCADE);

    // Tell PICs where to start their IRQs in CPU's IVT
    port_write_u8(PIC1_PORT_DATA, PIC1_VECTOR_BASE);
    port_write_u8(PIC2_PORT_DATA, PIC2_VECTOR_BASE);

    // Tell PICs about master/slave configuration
    port_write_u8(PIC1_PORT_DATA, PIC1_PIC2_IRQ);
    port_write_u8(PIC2_PORT_DATA, PIC2_CASCADE_ID);

    // Enable 8086 mode
    port_write_u8(PIC1_PORT_DATA, PIC_ICW4_8086);
    port_write_u8(PIC2_PORT_DATA, PIC_ICW4_8086);

    // Restore mask of active IRQs
    port_write_u8(PIC1_PORT_DATA, mask_1);
    port_write_u8(PIC2_PORT_DATA, mask_2);
}

void pic_disable(void) {
    port_write_u8(PIC1_PORT_DATA, PIC_CMD_DISABLE);
    port_write_u8(PIC2_PORT_DATA, PIC_CMD_DISABLE);
}

void pic_unmask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_PORT_DATA;
    } else if (irq >= 8 && irq < 16) {
        port = PIC2_PORT_DATA;
    } else {
        kpanicf("PIC: Attempt to unmask invalid IRQ");
    }
    const uint8_t mask = port_read_u8(port) & ~(1 << irq);
    port_write_u8(port, mask);
}

void pic_mask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_PORT_DATA;
    } else if (irq >= 8 && irq < 16) {
        port = PIC2_PORT_DATA;
    } else {
        kpanicf("PIC: Attempt to mask invalid IRQ");
    }
    const uint8_t mask = port_read_u8(port) | (1 << irq);
    port_write_u8(port, mask);
}

void pic_ack(uint8_t irq) {
    // If on PIC1 ack only there, otherwise on both
    if (irq >= 8) {
        port_write_u8(PIC2_PORT_CMD, PIC_CMD_EOI);
    }
    port_write_u8(PIC1_PORT_CMD, PIC_CMD_EOI);
}

bool pic_idt_is_managed(uint8_t idt_slot) {
    return (idt_slot >= PIC1_VECTOR_BASE) && idt_slot <= (PIC2_VECTOR_BASE + 7);
}

uint16_t pic_get_irr(void) {
    port_write_u8(PIC1_PORT_CMD, PIC_CMD_READ_IRR);
    port_write_u8(PIC2_PORT_CMD, PIC_CMD_READ_IRR);
    return (port_read_u8(PIC1_PORT_CMD) << 8) | port_read_u8(PIC2_PORT_CMD);
}

uint16_t pic_get_isr(void) {
    port_write_u8(PIC1_PORT_CMD, PIC_CMD_READ_ISR);
    port_write_u8(PIC2_PORT_CMD, PIC_CMD_READ_ISR);
    return (port_read_u8(PIC1_PORT_CMD) << 8) | port_read_u8(PIC2_PORT_CMD);
}
