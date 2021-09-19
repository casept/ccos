#include "../include/exception.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../../common.h"
#include "../include/interrupt.h"

#define EXCEPTION_NUM_VECTORS 32

enum exception_id_t {
    /// Divide-by-0
    EXCEPTION_DE = 0,
    /// Debug
    EXCEPTION_DB = 1,
    /// Non-maskable interrupt
    EXCEPTION_NMI = 2,
    /// Breakpoint
    EXCEPTION_BP = 3,
    /// Overflow
    EXCEPTION_OF = 4,
    /// Bound Range Exceeded
    EXCEPTION_BR = 5,
    /// Invalid Opcode
    EXCEPTION_UD = 6,
    /// Device Not Available
    EXCEPTION_NM = 7,
    /// Double Fault
    EXCEPTION_DF = 8,
    /// Coprocessor segment overrun (Can't happen on long mode)
    EXCEPTION_CSO = 9,
    /// Invalid TSS
    EXCEPTION_TS = 10,
    /// Segment Not Present
    EXCEPTION_NP = 11,
    /// Stack Segment Fault
    EXCEPTION_SS = 12,
    /// General Protection Fault
    EXCEPTION_GP = 13,
    /// Page Fault
    EXCEPTION_PF = 14,
    /// Reserved
    EXCEPTION_RESERVED_15 = 15,
    /// x87 FP Exception
    EXCEPTION_MF = 16,
    /// Alignment Check
    EXCEPTION_AC = 17,
    /// Machine Check
    EXCEPTION_MC = 18,
    /// SIMD FP
    EXCEPTION_XM = 19,
    /// Virtualization Exception
    EXCEPTION_VE = 20,
    /// Control Protection Exception
    EXCEPTION_CE = 21,
    /// Reserved
    EXCEPTION_RESERVED_22 = 22,
    /// Reserved
    EXCEPTION_RESERVED_23 = 23,
    /// Reserved
    EXCEPTION_RESERVED_24 = 24,
    /// Reserved
    EXCEPTION_RESERVED_25 = 25,
    /// Reserved
    EXCEPTION_RESERVED_26 = 26,
    /// Reserved
    EXCEPTION_RESERVED_27 = 27,
    /// Reserved
    EXCEPTION_RESERVED_28 = 28,
    /// Reserved
    EXCEPTION_RESERVED_29 = 29,
    /// Security Exception
    EXCEPTION_SXZ = 30,
    /// Reserved
    EXCEPTION_RESERVED_31 = 31,
};

bool EXCEPTION_HAS_ERRCODE[EXCEPTION_NUM_VECTORS] = {false, false, false, false, false, false, false, false, true,
                                                     false, true,  true,  true,  true,  true,  false, false, true,
                                                     false, false, false, false, true,  false, false, false};

const char* EXCEPTION_NAME[EXCEPTION_NUM_VECTORS] = {
    "Divide-by-zero",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved (15)",
    "x87 FP Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD FP Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved (22)",
    "Reserved (23)",
    "Reserved (24)",
    "Reserved (25)",
    "Reserved (26)",
    "Reserved (27)",
    "Reserved (28)",
    "Reserved (29)",
    "Security Exception",
    "Reserved (31)",
};

/// C component of the handler.
void exception_generic_handler_c(uint64_t exception_id, uint64_t errcode) {
    if (EXCEPTION_HAS_ERRCODE[(size_t)exception_id]) {
        kpanicf("Got unhandled exception '%c' with error code %u", EXCEPTION_NAME[(size_t)exception_id], errcode);
    } else {
        kpanicf("Got unhandled exception '%c'", EXCEPTION_NAME[(size_t)exception_id]);
    }
}

// ASM stubs
void exception_dbz(void);

void exception_register_handlers(void) { interrupt_register(exception_dbz, EXCEPTION_DE); }
