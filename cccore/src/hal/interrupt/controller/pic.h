#pragma once

#include <stdbool.h>
#include <stdint.h>

/// Enable the PICs.
void pic_enable(void);

/// Disable the PICs.
void pic_disable(void);

/// Mask (disable) a particular IRQ.
void pic_mask(uint8_t irq);

/// Unmask (enable) a particular IRQ.
void pic_unmask(uint8_t irq);

/// Acknowledge a particular IRQ.
void pic_ack(uint8_t irq);

/// Calculate which IDT slot corresponds to the IRQ.
uint8_t pic_irq_to_idt_slot(uint8_t irq);

/// Calculate which IRQ corresponds to the IDT slot, if any.
uint8_t pic_idt_slot_to_irq(uint8_t idt_slot);

/// Check whether the given IDT entry is managed by the PIC.
bool pic_idt_is_managed(uint8_t idt_slot);
