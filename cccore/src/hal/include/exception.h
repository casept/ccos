#pragma once

/// Register default exception handlers which
/// provide debug information before halting the machine.
///
/// Interrupts must be initialized before calling this.
void exception_register_default(void);
