#pragma once

#include <stdarg.h>
#include <stdint.h>

//! Abstraction layer for the legacy serial port.

/// The serial baud rate supported by the driver.
extern const uint32_t SERIAL_BAUD_RATE;

/// Initializes COM1 and registers an interrupt handler for it.
/// Interrupts must be enabled for serial comms to work.
void serial_com1_init(void);

/// Outputs a single character over COM1, once there's space in the TX buffer.
void serial_com1_write(uint8_t data);

/// Outputs a formatted string over COM1.
void serial_com1_vprintf(const char* format, va_list vlist);

/// Outputs a formatted string over COM1.
void serial_com1_printf(const char* format, ...);
