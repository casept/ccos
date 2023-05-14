#pragma once

#include <stdint.h>

/// Data that is passed by the ASM stubs to all C ISRs.
///
/// Depending on the interrupt, some fields may contain dummy values.
///
/// The order is important, as the CPU and ASM stubs will push the registers in this order.
struct __attribute__((__packed__)) interrupt_isr_data_t {
    // Machine registers
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    // Data added by CPU or ASM ISR stub
    uint64_t int_num;
    uint64_t int_arg;

    // Data added by CPU
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

/// Output the contents of an interrupt_isr_data_t to debug output.
void kprintf_interrupt_isr_data_t(const struct interrupt_isr_data_t* data);

/// Signature that all custom ISRs must obey.
///
/// Writing to the struct will modify the respective register on ISR exit.
typedef void (*interrupt_isr_t)(struct interrupt_isr_data_t*);

/// Initialize relevant data structures and hardware for processing interrupts.
///
/// Also register default handlers for all interrupts and CPU exceptions.
/// These simply log the occurrence to debug output and halt the machine.
void interrupt_init(void);

/// Register a non-default interrupt handler.
void interrupt_register(interrupt_isr_t isr, uint8_t idt_slot);

/// Acknowledge the interrupt to whatever interrupt controller underlies it.
///
/// Your non-default interrupt handlers must call this if appropriate.
/// Do not call this for interrupts that are CPU-internal (AKA exceptions), which don't have a controller.
void interrupt_ack(uint8_t idt_slot);

/// Enable interrupts.
void interrupt_enable(void);

/// Disable interrupts.
void interrupt_disable(void);
