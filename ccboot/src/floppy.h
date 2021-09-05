#pragma once

//! Protected mode floppy driver.
//! While not quite as hardware-agnostic as using the BIOS,
//! at least loading this way doesn't require writing and debugging real mode assembly,
//! which I don't feel like doing right now.
//!
//! This driver uses programmed I/O over DMA for the sake of simplicity, and because efficient CPU use isn't really a
//! priority when the only program that runs is the bootloader anyways.
//!
//! Also, may or may not work on real hardware (but it works in QEMU (TM), so good enough for now).
//! This code does not like multiple floppy controlles or multiple drives, though.
//! Also assumes 1.44MB floppy disks.

#include <stddef.h>
#include <stdint.h>

/// Reads from the first floppy drive.
///
/// Does not bounds check the destination buffer.
void floppy_read(size_t lba_start_sector_idx, size_t num_sectors, uint8_t* dest);
