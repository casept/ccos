#pragma once

#include <stdint.h>

/// Type for the function that will be executed on timer expiration.
typedef void (*timer_callback_t)(void);

/// Enables a timer with the closest tickrate
/// to the desired rate that the timer supports.
///
/// Interrupt subsystem must be initialized before calling this.
///
/// Also, interrupts must be enabled for the timer to work.
void timer_enable(uint32_t tickrate_hz, timer_callback_t callback);
