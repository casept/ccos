#pragma once

#include <stdint.h>

/// Initialize relevant data structures and hardware for processing exceptions.
///
/// Also register default handlers for all interrupts and CPU exceptions.
/// These simply log the occurrence to debug output and halt the machine.
void interrupt_init(void);

/// Signature that all custom ISRs must obey.
typedef void (*isr_t)(void);

/// Register a non-default interrupt handler.
void interrupt_register(isr_t isr, uint8_t idt_slot);

/// Acknowledge the interrupt to whatever interrupt controller underlies it.
///
/// Your non-default interrupt handlers must call this if appropriate.
/// Do not call this for interrupts that are CPU-internal (AKA exceptions), which don't have a controller.
void interrupt_ack(uint8_t idt_slot);

/// Enable interrupts.
void interrupt_enable(void);

/// Disable interrupts.
void interrupt_disable(void);
