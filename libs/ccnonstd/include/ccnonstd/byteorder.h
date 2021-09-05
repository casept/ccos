#pragma once

//! The fact that this header has to exist should be an embarrasment for the C standard.

#include <stdbool.h>
#include <stdint.h>

/// Determine machine endianness.
/// Return `false` for big endian, `true` for little endian.
///
/// Return value on mixed-endian architectures is undefined.
bool byteorder(void);

/// Read given big-endian byte buffer into a `uint16_t` of native endianness.
uint16_t byteorder_be_to_u16_native(const uint8_t buf[2]);
/// Read given little-endian byte buffer into a `uint16_t` of native endianness.
uint16_t byteorder_le_to_u16_native(const uint8_t buf[2]);
/// Wrapper for big-endian and little-endian to native functions
/// which calls the big-endian variant if first argument is `false`,
/// or the little-endian variant if first argument is `true`.
uint16_t byteorder_to_u16_native(bool endianness, const uint8_t buf[2]);

/// Read given big-endian byte buffer into a `uint32_t` of native endianness.
uint32_t byteorder_be_to_u32_native(const uint8_t buf[4]);
/// Read given little-endian byte buffer into a `uint32_t` of native endianness.
uint32_t byteorder_le_to_u32_native(const uint8_t buf[4]);
/// Wrapper for big-endian and little-endian to native functions
/// which calls the big-endian variant if first argument is `false`,
/// or the little-endian variant if first argument is `true`.
uint32_t byteorder_to_u32_native(bool endianness, const uint8_t buf[4]);

/// Read given big-endian byte buffer into a `uint32_t` of native endianness.
uint64_t byteorder_be_to_u64_native(const uint8_t buf[8]);
/// Read given little-endian byte buffer into a `uint32_t` of native endianness.
uint64_t byteorder_le_to_u64_native(const uint8_t buf[8]);
/// Wrapper for big-endian and little-endian to native functions
/// which calls the big-endian variant if first argument is `false`,
/// or the little-endian variant if first argument is `true`.
uint64_t byteorder_to_u64_native(bool endianness, const uint8_t buf[8]);
