#pragma once

#include <stdint.h>

/// Initialize relevant data structures and hardware for processing interrupts.
void interrupt_init(void);

/// Signature that all ISRs must obey.
typedef void (*isr_t)(void);

/// Registers a new interrupt handler.
void interrupt_register(isr_t isr, uint8_t idt_slot);

/// Acknowledges the interrupt to whatever interrupt controller underlies it.
void interrupt_ack(uint8_t idt_slot);

/// Enables interrupts.
void interrupt_enable(void);

/// Disables interrupts.
void interrupt_disable(void);
