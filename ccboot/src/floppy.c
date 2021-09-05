#include "floppy.h"

#include <ccvga.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "io_port.h"

static const size_t FLOPPY_SECTORS_PER_TRACK = 18;
static const size_t FLOPPY_SECTOR_SIZE = 512;
static const uint8_t FLOPPY_DRIVE_NUM = 0;
static const uint8_t FLOPPY_CONTROLLER_SUPPORTED_VERSION = 0x90;

/// I/O ports of use for interacting with the floppy.
enum floppy_ports_t {
    FLOPPY_PORT_DOR = 0x03F2,
    FLOPPY_PORT_MSR = 0x03F4,
    FLOPPY_PORT_DATA_FIFO = 0x03F5,
};

/// Commands that can be issued to the drive.
enum floppy_commands_t {
    FLOPPY_COMMAND_RECALIBRATE = 7,
    FLOPPY_COMMAND_SENSE_INTERRUPT = 8,
    FLOPPY_COMMAND_SEEK = 15,
    FLOPPY_COMMAND_VERSION = 16,
    FLOPPY_COMMAND_CONFIGURE = 19,
    FLOPPY_COMMAND_LOCK = 148,
};

/// Bits making up the main status register
enum floppy_msr_bits_t {
    FLOPPY_MSR_BIT_ACTA = 0x00,
    FLOPPY_MSR_BIT_ACTB = 0x01,
    FLOPPY_MSR_BIT_ACTC = 0x02,
    FLOPPY_MSR_BIT_ACTD = 0x04,
    FLOPPY_MSR_BIT_CB = 0x10,
    FLOPPY_MSR_BIT_NDMA = 0x20,
    FLOPPY_MSR_BIT_DIO = 0x40,
    FLOPPY_MSR_BIT_RQM = 0x80,
};

/// Bits making up the digital output register
enum floppy_dor_bits_t {
    FLOPPY_DOR_BIT_RESET = 0x04,
    FLOPPY_DOR_BIT_IRQ = 0x08,
};

/// Reset the floppy controller.
static void floppy_controller_cmd_reset(void) {
    // Cannot use the `floppy_controller_send_command` function because it requires this function for retries, creating
    // a circular dependency.
    port_write_u8(FLOPPY_PORT_DOR, 0x00);
    // TODO: Real hardware requires 4us wait here
    const uint8_t dor = FLOPPY_DOR_BIT_RESET;
    port_write_u8(FLOPPY_PORT_DOR, dor);

    // TODO: Because we're in polling mode, send 4 sense interrupt commands

    // TODO: Re-send configure commands
}

static uint8_t floppy_lba_2_cylinder(size_t lba) { return lba / (2 * FLOPPY_SECTORS_PER_TRACK); }

static uint8_t floppy_lba_2_head(size_t lba) {
    return ((lba % (2 * FLOPPY_SECTORS_PER_TRACK)) / FLOPPY_SECTORS_PER_TRACK);
}

static uint8_t floppy_lba_2_sector(size_t lba) {
    return ((lba % (2 * FLOPPY_SECTORS_PER_TRACK)) % FLOPPY_SECTORS_PER_TRACK + 1);
}

/// Send a command or parameter byte to the floppy controller.
static int floppy_controller_send_byte(uint8_t byte) {
    // Send command and wait for ack
    port_write_u8(FLOPPY_PORT_DATA_FIFO, byte);
    while ((port_read_u8(FLOPPY_PORT_MSR) & FLOPPY_MSR_BIT_RQM) == 0) {
    }
    // Must verify that DIO == 0
    if ((port_read_u8(FLOPPY_PORT_MSR) & FLOPPY_MSR_BIT_DIO) != 0) {
        return -1;
    }

    return 0;
}

/// Send a command to the primary floppy controller.
///
/// Commands that require parameter bytes must pass these in using `param_bytes`.
/// If `param_bytes` is NULL, no parameter bytes will be passed.
///
/// Commands that return result bytes will have these written to `result_bytes`.
/// Remember to set `num_result_bytes` and size your buffer accordingly.
///
/// Command will be retried as indicated by `num_retries`.
/// If the final retry also fails, returns a negative error code.
/// Otherwise, returns `0`.
static int floppy_controller_send_command(enum floppy_commands_t cmd, const uint8_t* param_bytes,
                                          size_t num_param_bytes, uint8_t* result_bytes, size_t num_result_bytes,
                                          size_t num_retries) {
    int err = 0;
    for (size_t i = 0; i <= num_retries; i++) {
        // Check whether previous command didn't receive a fatal error
        const uint8_t status = port_read_u8(FLOPPY_PORT_MSR);
        vga_printf("MSR: 0b%b\n", (unsigned int)status);
        if ((status & FLOPPY_MSR_BIT_RQM) == 0) {
            err = -1;
            floppy_controller_cmd_reset();
            continue;
        }
        if ((status & FLOPPY_MSR_BIT_DIO) != 0) {
            err = -2;
            floppy_controller_cmd_reset();
            continue;
        }

        // Send command byte
        if (floppy_controller_send_byte(cmd) == -1) {
            err = -3;
            floppy_controller_cmd_reset();
            continue;
        }

        // Send parameter bytes
        if (param_bytes != NULL) {
            for (size_t j = 0; j < num_param_bytes; j++) {
                if (floppy_controller_send_byte(param_bytes[j]) == -1) {
                    // TODO: Continue outer loop
                    floppy_controller_cmd_reset();
                    return -4;
                }
            }
        }

        // Wait for execution phase to end and result phase to begin
        while (true) {
            const uint8_t exec_status = port_read_u8(FLOPPY_PORT_MSR);
            if ((exec_status & FLOPPY_MSR_BIT_NDMA) != 0) {
                // No execution phase; proceed directly to result
                break;
            }
            // TODO: Other waiting
        }

        // Read result bytes
        if (result_bytes != NULL) {
            for (size_t j = 0; j < num_result_bytes; j++) {
                // TODO: Error/overflow/underflow handling
                result_bytes[j] = port_read_u8(FLOPPY_PORT_DATA_FIFO);
            }
        }

        // Ensure everyting looks OK
        const uint8_t end_status = port_read_u8(FLOPPY_PORT_MSR);
        if ((end_status & FLOPPY_MSR_BIT_RQM) == 0) {
            err = -5;
            floppy_controller_cmd_reset();
            continue;
        }
        if ((end_status & FLOPPY_MSR_BIT_CB) != 0) {
            err = -6;
            floppy_controller_cmd_reset();
            continue;
        }
        if ((end_status & FLOPPY_MSR_BIT_DIO) != 0) {
            err = -7;
            floppy_controller_cmd_reset();
            continue;
        }

        return 0;
    }

    return err;
}

static void floppy_controller_cmd_configure(void) {
    // Implied seek enabled, threshold 8
    const uint8_t params[3] = {0x00, (1 << 6) | 8, 0x00};
    floppy_controller_send_command(FLOPPY_COMMAND_CONFIGURE, params, 3, NULL, 0, 5);
}

static void floppy_controller_cmd_lock_config(void) {
    uint8_t result;
    floppy_controller_send_command(FLOPPY_COMMAND_LOCK, NULL, 0, &result, 1, 5);
}

static void floppy_controller_cmd_recalibrate(void) {
    const uint8_t params[1] = {FLOPPY_DRIVE_NUM};
    floppy_controller_send_command(FLOPPY_COMMAND_RECALIBRATE, params, 1, NULL, 0, 5);

    // Spin until drives no longer busy
    while (true) {
        const uint8_t msr = port_read_u8(FLOPPY_PORT_MSR);
        if ((msr & (FLOPPY_MSR_BIT_ACTA | FLOPPY_MSR_BIT_ACTB | FLOPPY_MSR_BIT_ACTC | FLOPPY_MSR_BIT_ACTD)) == 0) {
            break;
        }
    }
    // TODO: Sense interrupt
}

static void floppy_controller_init(void) {
    // Ensure controller version is not pre-prehistoric or from the future
    uint8_t version;
    const int err = floppy_controller_send_command(FLOPPY_COMMAND_VERSION, NULL, 0, &version, 1, 5);
    if (err != 0) {
        vga_fatalf("could not read floppy: command to determine controller version failed (error code: %d)", err);
    }
    if (version != FLOPPY_CONTROLLER_SUPPORTED_VERSION) {
        vga_fatalf("could not read floppy: controller version is not supported (expected 0x%x, got 0x%x)",
                   (unsigned int)FLOPPY_CONTROLLER_SUPPORTED_VERSION, (unsigned int)version);
    }

    // Send configuration
    floppy_controller_cmd_configure();

    // Lock it in so it survives controller resets
    floppy_controller_cmd_lock_config();

    // Actual reset
    floppy_controller_cmd_reset();

    // Recalibrate the drive
    floppy_controller_cmd_recalibrate();
}

static void floppy_controller_wait_drive_inactive(void) {
    const uint8_t msr = port_read_u8(FLOPPY_PORT_MSR);
    while ((msr & (1 << FLOPPY_DRIVE_NUM)) == 0) {
    }
}

/// Sends a sense interrupt command to the drive.
///
/// Returns st0.
static uint8_t floppy_controller_cmd_sense_interrupt(void) {
    uint8_t result[2];
    if (floppy_controller_send_command(FLOPPY_COMMAND_SENSE_INTERRUPT, NULL, 0, result, 2, 5) == -1) {
        vga_fatalf("could not read floppy: failed to send sense interrupt");
    }
    return result[0];
}

static void floppy_controller_cmd_seek(size_t lba) {
    const uint8_t c = floppy_lba_2_cylinder(lba);
    const uint8_t h = floppy_lba_2_head(lba);
    const uint8_t params[2] = {(h << 2) | FLOPPY_DRIVE_NUM, c};
    if (floppy_controller_send_command(FLOPPY_COMMAND_SEEK, params, 2, NULL, 0, 5) == -1) {
        vga_fatalf("could not read floppy: failed to submit seek command");
    }
    // Head movement is done once drive is inactive
    floppy_controller_wait_drive_inactive();
    // Check for errors
    const uint8_t st0 = floppy_controller_cmd_sense_interrupt();
    if ((st0 & 0xC0) > 0) {
        vga_fatalf("could not read floppy: seek command reported failure");
    }
}

static void floppy_controller_read(size_t lba, uint8_t* dest) {
    // Seek to correct cylinder
    floppy_controller_cmd_seek(lba);
    // TODO: Issue read command
    // TODO: Poll until done
    // TODO: Check for errors and retry
    (void)dest;
}

void floppy_read(size_t lba_start_sector_idx, size_t num_sectors, uint8_t* dest) {
    floppy_controller_init();
    for (size_t i = 0; i < num_sectors; i++) {
        floppy_controller_read(lba_start_sector_idx + i, dest);
        dest += FLOPPY_SECTOR_SIZE;
    }
    (void)floppy_lba_2_sector;
}
