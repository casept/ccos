#pragma once

/// Initialize the kernel console.
void kprint_init(void);

/// Print message to console.
void kprintf(const char* format, ...);

/// Print message to console and halt system.
void kpanicf(const char* format, ...);
