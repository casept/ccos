#include <stdbool.h>
#include <stddef.h>

/// Returns true if lhs and rhs are bitwise identical, false otherwise.
bool memcmp_bool(const void* lhs, const void* rhs, size_t count);

/// Like memset, but for volatile destinations.
volatile void* memset_volatile(volatile void* dest, int ch, size_t count);
