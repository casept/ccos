#include "../include/interrupt.h"

#include <stddef.h>
#include <stdint.h>

#include "../../common.h"
#include "controller/pic.h"
#include "gdt.h"

/// Selector that tells CPU how to look up a segment for an IDT.
typedef uint16_t idt_selector_t;

/// Interrupt descriptor table entry telling the CPU how to handle a particular interrupt.
struct __attribute__((__packed__)) idt_entry_t {
    uint16_t offset_1;
    idt_selector_t selector;
    uint8_t ist;
    uint8_t type_and_attr;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t reserved_zeroed;
};

/// Descriptor telling the CPU where to find the IDT.
struct __attribute__((__packed__)) idt_info_t {
    uint16_t size;    // Table's size (in number of bytes - 1)
    uint64_t offset;  // Table's virtual address
};

// Attributes for IDT entries.
static const uint8_t TYPE_ATTR_INTERRUPT_64 = 0b00001110;
static const uint8_t TYPE_ATTR_INTERRUPT_PRESENT = 0b10000000;
static const uint8_t TYPE_ATTR_INTERRUPT_PROTECTION_RING0 = 0b00000000;

// Flags for an IDT selector.
static const idt_selector_t SELECTOR_PRIVILEGE_RING0 = 0b0000000000000000;
static const idt_selector_t SELECTOR_TABLE_GDT = 0b0000000000000000;

#define IDT_NUM_ENTRIES 256
static __attribute__((aligned(16))) struct idt_entry_t IDT[IDT_NUM_ENTRIES];
static __attribute__((aligned(16))) __attribute__((used)) struct idt_info_t IDT_INFO = {
    .size = (IDT_NUM_ENTRIES * sizeof(struct idt_entry_t)) - 1,
    .offset = (uint64_t)IDT,
};

/// This table stores C interrupt handlers, if defined.
static isr_t INT_HANDLERS[IDT_NUM_ENTRIES];

/// This table stores addresses of ASM stubs.
/// These are actually written into the IDT.
extern uintptr_t ISR_TABLE[IDT_NUM_ENTRIES];

/// This structure describes the order in which the ASM interrupt service routine pushes registers and other data onto
/// the stack. It's defined to allow easy access to this data.
struct __attribute__((__packed__)) isr_data_t {
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
};

/// Construct a selector into the GDT for use in an IDT entry.
static idt_selector_t idt_selector_create(void) { return SELECTOR_PRIVILEGE_RING0 | SELECTOR_TABLE_GDT | GDT_CODE_IDX; }

/// Register the ASM trampoline for a particular ISR in the IDT.
/// This is only called once for each IDT slot.
static void idt_register(uintptr_t asm_isr, uint8_t idt_slot) {
    const uint64_t isr_addr = (uint64_t)asm_isr;
    // FIXME: Faults need different gate
    const struct idt_entry_t d = {
        .offset_1 = (uint16_t)(isr_addr & 0x000000000000FFFF),
        .offset_2 = (uint16_t)((isr_addr & 0x00000000FFFF0000) >> 16),
        .offset_3 = (uint32_t)((isr_addr & 0xFFFFFFFF00000000) >> 32),
        // Interrupt stack table unused for now
        .ist = 0,
        .type_and_attr = TYPE_ATTR_INTERRUPT_64 | TYPE_ATTR_INTERRUPT_PRESENT | TYPE_ATTR_INTERRUPT_PROTECTION_RING0,
        .reserved_zeroed = 0,
        .selector = idt_selector_create(),
    };

    IDT[idt_slot] = d;

    // If the interrupt falls within the range administered by the PIC, program it as well
    // TODO: Is this really the most appropriate place for this?
    if (pic_idt_is_managed(idt_slot)) {
        pic_unmask(pic_idt_slot_to_irq(idt_slot));
    };
}

void interrupt_init(void) {
    // Initialize entire table with the ASM stub for each interrupt
    for (size_t i = 0; i < IDT_NUM_ENTRIES; i++) {
        idt_register(ISR_TABLE[i], (uint8_t)i);
    };

    // No C handlers have been defined yet; clear the table
    for (size_t i = 0; i < IDT_NUM_ENTRIES; i++) {
        INT_HANDLERS[i] = NULL;
    };

    __asm__ volatile(
        ".intel_syntax noprefix  \n\t"
        "lidt [IDT_INFO]         \n\t"
        ".att_syntax prefix      \n\t");

    // For now, we only support legacy PIC. Therefore, initialize it unconditionally.
    pic_enable();
    // TODO: Install handler for spurious PIC interrupts
}

void interrupt_register(isr_t isr, uint8_t idt_slot) { INT_HANDLERS[(size_t)idt_slot] = isr; }

void interrupt_enable(void) { __asm__ volatile("sti"); }

void interrupt_disable(void) { __asm__ volatile("cli"); }

void interrupt_ack(uint8_t idt_slot) {
    if (pic_idt_is_managed(idt_slot)) {
        pic_ack(pic_idt_slot_to_irq(idt_slot));
    } else {
        kpanicf("interrupt_ack(): Can't ack because interrupt controller for IDT slot %u is unknown.", idt_slot);
    }
}

/// Called by ASM to dispatch interrupts to the appropriate C handler registered in `INT_HANDLERS`.
/// TODO: Implement
void isr_dispatch(struct isr_data_t* data) {
    // Do we have a registered C ISR for this?
    isr_t handler = INT_HANDLERS[(size_t)data->int_num];
    if (handler != NULL) {
        handler();
    } else {
        kpanicf("Got unhandled interrupt number %u with argument %u", data->int_num, data->int_arg);
    }
}
