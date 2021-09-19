#include "include/interrupt.h"

#include <stddef.h>
#include <stdint.h>

#include "../../common.h"
#include "include/gdt.h"
#include "pic.h"

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
    uint16_t size;    // Table's size (in number of entries)
    uint64_t offset;  // Table's virtual address
};

// Attributes for IDT entries
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

/// This is the default ISR, called when the specific one has not been installed.
/// It prints a message and halts execution.
///
/// Implemented in the accompanying ASM file.
void interrupt_default_isr(void);

/// C component of the default ISR.
void interrupt_default_isr_c(void) { kpanicf("Encountered interrupt for which no ISR has been provided"); }

/// Construct a selector into the GDT.
static idt_selector_t interrupt_selector_create(void) {
    idt_selector_t s = SELECTOR_PRIVILEGE_RING0 | SELECTOR_TABLE_GDT;
    // Index into the table occupies bit 3..15
    s |= (GDT_CODE_IDX << 3);
    return s;
}

// TODO: TODO: GPF (13), read selector error code (from stack?)
// TODO: TODO: Is fetch_raw_descriptor: GDT: index (47) 8 > limit (17) related?
// TODO: TODO: That limit looks weird (not n*8 -1)
void interrupt_register(isr_t isr, uint8_t idt_slot) {
    const uint64_t isr_addr = (uint64_t)isr;
    const struct idt_entry_t d = {
        .offset_1 = (uint16_t)(isr_addr & 0x000000000000FFFF),
        .offset_2 = (uint16_t)((isr_addr & 0x00000000FFFF0000) >> 16),
        .offset_3 = (uint32_t)((isr_addr & 0xFFFFFFFF00000000) >> 32),
        // Interrupt stack table unused for now
        .ist = 0,
        .type_and_attr = TYPE_ATTR_INTERRUPT_64 | TYPE_ATTR_INTERRUPT_PRESENT | TYPE_ATTR_INTERRUPT_PROTECTION_RING0,
        .reserved_zeroed = 0,
        .selector = interrupt_selector_create(),
    };

    IDT[idt_slot] = d;

    // If the interrupt falls within the range administered by the PIC, program it as well
    if (pic_idt_is_managed(idt_slot)) {
        pic_unmask(pic_idt_slot_to_irq(idt_slot));
    };
}

// FIXME:  interrupt(long mode): not accessible or not code segment
// FIXME: Faults need different gate
void interrupt_init(void) {
    // Initialize entire table to default handler
    for (uint8_t slot = 0; slot < (IDT_NUM_ENTRIES - 1); slot++) {
        interrupt_register(interrupt_default_isr, slot);
    };

    __asm__ volatile(
        ".intel_syntax noprefix  \n\t"
        "lidt [IDT_INFO]         \n\t"
        ".att_syntax prefix      \n\t");

    // For now, we only support legacy PIC. Therefore, initialize it unconditionally.
    pic_enable();
    // TODO: Install handler for spurious PIC interrupts
}

void interrupt_enable(void) { __asm__ volatile("sti"); }

void interrupt_disable(void) { __asm__ volatile("cli"); }

void interrupt_ack(uint8_t idt_slot) {
    if (pic_idt_is_managed(idt_slot)) {
        pic_ack(pic_idt_slot_to_irq(idt_slot));
    } else {
        kpanicf("interrupt_ack(): Can't ack because interrupt controller for IDT slot %u is unknown.", idt_slot);
    }
}
