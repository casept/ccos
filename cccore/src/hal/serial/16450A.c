#include <ccnonstd/io.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "../../common.h"
#include "../include/interrupt.h"
#include "../include/io_port.h"
#include "../include/serial.h"
#include "../interrupt/controller/pic.h"

/* Based on https://wiki.osdev.org/Serial_Ports */

const uint32_t SERIAL_BAUD_RATE = 300;

/// Speed of the hardware clock.
static const uint32_t HW_BAUD_RATE = 115200;

/// IRQ number for COM1.
static const uint8_t COM1_IRQ = 4;

/// NOTE: Not necessarily standardized
static const uint16_t COM1_PORT_BASE = 0x3F8;

/// Offsets from base port for controlling various features.
static const uint16_t OFFSET_DATA = 0;
static const uint16_t OFFSET_INTERRUPTS = 1;
static const uint16_t OFFSET_DIVISOR_LSB = 0;
static const uint16_t OFFSET_DIVISOR_MSB = 1;
static const uint16_t OFFSET_FIFO_CTL = 2;
static const uint16_t OFFSET_LINE_CTL = 3;
static const uint16_t OFFSET_MODEM_CTL = 4;
static const uint16_t OFFSET_LINE_STATUS = 5;

/// Interrupts that can be enabled on the serial controller.
enum irq_ctl_t {
    IRQ_CTL_DATA_AVAILABLE = 0b00000001,
};

/// Values that can be set in the line control register.
enum line_ctl_t {
    LINE_CTL_DLAB = 0b10000000,
    LINE_CTL_PARITY_NONE = 0b00000000,
    LINE_CTL_STOP_1 = 0b00000000,
    LINE_CTL_DATA_8 = 0b00000011,
};

/// Values that can be set in the FIFO control register.
enum fifo_ctl_t {
    FIFO_CTL_ENABLE = 0b00000001,
    FIFO_CTL_IRQ_TRIGGER_LEVEL_8 = 0b10000000,
};

/// Values that can be set in the modem control register.
enum modem_ctl_t {
    MODEM_CTL_DTR = 0b00000001,
    MODEM_CTL_RTS = 0b00000010,
    MODEM_CTL_OUT1 = 0b00000100,
    MODEM_CTL_OUT2 = 0b00001000,
    MODEM_CTL_LOOPBACK = 0b00010000,
};

/// Values that can be set/read from the line status register.
enum line_status_t {
    LINE_STATUS_DATA_READY = 0b00000001,
    LINE_STATUS_TXBUF_READY = 0b00100000,
};

/// Calculates the serial clock divisor for a given baud rate.
static uint16_t baud_2_divisor(uint32_t baud) { return HW_BAUD_RATE / baud; }

static void irq_disable(void) { port_write_u8(COM1_PORT_BASE + OFFSET_INTERRUPTS, 0x00); }

static void divisor_set(void) {
    port_write_u8(COM1_PORT_BASE + OFFSET_LINE_CTL, LINE_CTL_DLAB);  // Enable DLAB
    const uint16_t divisor = baud_2_divisor(SERIAL_BAUD_RATE);
    port_write_u8(COM1_PORT_BASE + OFFSET_DIVISOR_LSB, (uint8_t)(divisor & 0x00FF));
    port_write_u8(COM1_PORT_BASE + OFFSET_DIVISOR_MSB, (uint8_t)((divisor & 0xFF00) >> 8));
}

/// 8 bits, no parity, 1 stop bit
static void config_set_8n1(void) {
    const uint8_t config = LINE_CTL_PARITY_NONE | LINE_CTL_DATA_8 | LINE_CTL_STOP_1;
    port_write_u8(COM1_PORT_BASE + OFFSET_LINE_CTL, config);
}

static void fifo_enable(void) {
    const uint8_t fifo = FIFO_CTL_ENABLE | FIFO_CTL_IRQ_TRIGGER_LEVEL_8;
    port_write_u8(COM1_PORT_BASE + OFFSET_FIFO_CTL, fifo);
}

static uint8_t read(void) { return port_read_u8(COM1_PORT_BASE + OFFSET_DATA); }

static void write(uint8_t data) { port_write_u8(COM1_PORT_BASE + OFFSET_DATA, data); }

static bool self_test(void) {
    // Enable loopback
    port_write_u8(COM1_PORT_BASE + OFFSET_MODEM_CTL, MODEM_CTL_LOOPBACK);

    // Echo test
    write(0xAE);
    if (read() != 0xAE) {
        return false;
    }

    // Disable loopback
    port_write_u8(COM1_PORT_BASE + OFFSET_MODEM_CTL, 0x00);
    return true;
}

static void config_enable_out(void) {
    const uint8_t config = MODEM_CTL_DTR | MODEM_CTL_RTS | MODEM_CTL_OUT1 | MODEM_CTL_OUT2;
    port_write_u8(COM1_PORT_BASE + OFFSET_MODEM_CTL, config);
}

static void serial_read_isr(struct interrupt_isr_data_t* data);

static void config_set_irq(void) {
    // Register an ISR that deals with reading the data
    interrupt_register(serial_read_isr, pic_irq_to_idt_slot(COM1_IRQ));

    // Enable interrupt on data arrival
    const uint8_t config = IRQ_CTL_DATA_AVAILABLE;
    port_write_u8(COM1_PORT_BASE + OFFSET_INTERRUPTS, config);
}

void serial_com1_init(void) {
    irq_disable();
    divisor_set();
    config_set_8n1();
    fifo_enable();
    if (self_test() == false) {
        kpanicf("serial_com1_init(): Self-test failed");
    }
    config_enable_out();
    config_set_irq();
}

static bool can_read(void) {
    const uint8_t line_status = port_read_u8(COM1_PORT_BASE + OFFSET_LINE_STATUS);
    return (line_status & LINE_STATUS_DATA_READY) != 0;
}

static bool can_write(void) {
    const uint8_t line_status = port_read_u8(COM1_PORT_BASE + OFFSET_LINE_STATUS);
    return (line_status & LINE_STATUS_TXBUF_READY) != 0;
}

static void serial_read_isr(struct interrupt_isr_data_t* data) {
    (void)data;
    // TODO: Do we need to ACK to the serial controller?
    // ACK interrupt controller
    // Read as much data as exists
    while (can_read()) {
        // TODO: Push to ring buffer
        kprintf("%c", read());
    }
}

void serial_com1_write(uint8_t data) {
    // TODO: Switch to interrupt-based writing
    // Wait for write to be possible
    while (!can_write()) {
    }
    port_write_u8(COM1_PORT_BASE + OFFSET_DATA, data);
}

static void com1_putc(char c) { serial_com1_write((uint8_t)c); }

void serial_com1_vprintf(const char* format, va_list vlist) {
    if (vprintf_generic(com1_putc, format, vlist) == -1) {
        kpanicf("serial_com1_vprintf(): Failed");
    }
}

void serial_com1_printf(const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    serial_com1_vprintf(format, vlist);
    va_end(vlist);
}
