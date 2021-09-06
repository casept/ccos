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
    FLOPPY_COMMAND_RECALIBRATE = 0x07,
    FLOPPY_COMMAND_SENSE_INTERRUPT = 0x08,
    FLOPPY_COMMAND_SEEK = 0x0F,
    FLOPPY_COMMAND_VERSION = 0x10,
    FLOPPY_COMMAND_CONFIGURE = 0x13,
    FLOPPY_COMMAND_LOCK = 0x94,
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

/* Pre-declaration of functions that have circular dependency on each other being declared */
static int send_command(enum floppy_commands_t cmd, const uint8_t* param_bytes, size_t num_param_bytes,
                        uint8_t* result_bytes, size_t num_result_bytes, size_t num_retries);
static uint8_t cmd_sense_interrupt(void);
static void cmd_configure(void);

/// Reset the floppy controller.
static void cmd_reset(void) {
    port_write_u8(FLOPPY_PORT_DOR, 0x00);
    port_write_u8(FLOPPY_PORT_DOR, 0x0C);

    // Because we're in polling mode, send 4 sense interrupt commands
    for (size_t i = 0; i < 4; i++) {
        cmd_sense_interrupt();
    }

    // Re-send configuration
    cmd_configure();
}

static uint8_t lba_2_cylinder(size_t lba) { return lba / (2 * FLOPPY_SECTORS_PER_TRACK); }

static uint8_t lba_2_head(size_t lba) { return ((lba % (2 * FLOPPY_SECTORS_PER_TRACK)) / FLOPPY_SECTORS_PER_TRACK); }

static uint8_t lba_2_sector(size_t lba) {
    return ((lba % (2 * FLOPPY_SECTORS_PER_TRACK)) % FLOPPY_SECTORS_PER_TRACK + 1);
}

static void wait_drive_inactive(void) {
    while ((port_read_u8(FLOPPY_PORT_MSR) & (1 << FLOPPY_DRIVE_NUM)) == 0) {
    }
}

/// Send a command or parameter byte to the floppy controller.
static int send_byte(uint8_t byte) {
    // Send command and wait for ack
    port_write_u8(FLOPPY_PORT_DATA_FIFO, byte);
    while ((port_read_u8(FLOPPY_PORT_MSR) & FLOPPY_MSR_BIT_RQM) == 0) {
    };

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
static int send_command(enum floppy_commands_t cmd, const uint8_t* param_bytes, size_t num_param_bytes,
                        uint8_t* result_bytes, size_t num_result_bytes, size_t num_retries) {
    int err = 0;
    for (size_t i = 0; i <= num_retries; i++) {
        // Check whether previous command didn't receive a fatal error
        const uint8_t status = port_read_u8(FLOPPY_PORT_MSR);
        if ((status & FLOPPY_MSR_BIT_RQM) == 0) {
            err = -1;
            cmd_reset();
            continue;
        }
        if ((status & FLOPPY_MSR_BIT_DIO) != 0) {
            err = -2;
            cmd_reset();
            continue;
        }

        // Send command byte
        if (send_byte(cmd) == -1) {
            err = -3;
            cmd_reset();
            continue;
        }

        // Send parameter bytes
        if (param_bytes != NULL) {
            for (size_t j = 0; j < num_param_bytes; j++) {
                if (send_byte(param_bytes[j]) == -1) {
                    err = -4;
                    cmd_reset();
                    goto continue_retry_loop;
                }
            }
        }

        // Wait for execution phase to end and result phase to begin
        if ((port_read_u8(FLOPPY_PORT_MSR) & FLOPPY_MSR_BIT_NDMA) == 0) {
            // No execution phase; proceed directly to result phase
            break;
        }
        while (true) {
            if ((port_read_u8(FLOPPY_PORT_MSR) & FLOPPY_MSR_BIT_RQM) != 0) {
                // TODO: Make work for commands which requires data transfer during execution phase
                break;
            }
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
            cmd_reset();
            continue;
        }
        if ((end_status & FLOPPY_MSR_BIT_CB) != 0) {
            err = -6;
            cmd_reset();
            continue;
        }
        if ((end_status & FLOPPY_MSR_BIT_DIO) != 0) {
            err = -7;
            cmd_reset();
            continue;
        }

        return 0;
    continue_retry_loop:;
    }

    return err;
}

static void cmd_configure(void) {
    // Implied seek enabled, FIFO threshold 8, IRQ and DMA disabled
    const uint8_t params[3] = {0x00, (1 << 6) | 8, 0x00};
    const int err = send_command(FLOPPY_COMMAND_CONFIGURE, params, 3, NULL, 0, 5);
    if (err != 0) {
        vga_fatalf("could not read floppy: failed to configure controller (error code: %d)", err);
    }
}

static void cmd_lock_config(void) {
    uint8_t result;
    const int err = send_command(FLOPPY_COMMAND_LOCK, NULL, 0, &result, 1, 5);
    if (err != 0) {
        vga_fatalf("could not read floppy: failed to lock controller configuration (error code: %d)", err);
    }
}

static void cmd_recalibrate(void) {
    const uint8_t params[1] = {FLOPPY_DRIVE_NUM};
    const int err = send_command(FLOPPY_COMMAND_RECALIBRATE, params, 1, NULL, 0, 5);
    if (err != 0) {
        vga_fatalf("could not read floppy: failed to recalibrate controller (error code: %d)", err);
    }

    wait_drive_inactive();
    // TODO: Sense interrupt
}

static void init(void) {
    vga_printf("reset\n");
    cmd_reset();
    vga_printf("configure\n");
    cmd_configure();
    vga_printf("lock_config\n");
    cmd_lock_config();
    // Ensure controller version is not pre-prehistoric or from the future
    uint8_t version;
    const int err = send_command(FLOPPY_COMMAND_VERSION, NULL, 0, &version, 1, 5);
    if (err != 0) {
        vga_fatalf("could not read floppy: command to determine controller version failed (error code: %d)", err);
    }
    if (version != FLOPPY_CONTROLLER_SUPPORTED_VERSION) {
        vga_fatalf("could not read floppy: controller version is not supported (expected 0x%x, got 0x%x)",
                   (unsigned int)FLOPPY_CONTROLLER_SUPPORTED_VERSION, (unsigned int)version);
    }

    cmd_configure();
    cmd_lock_config();
    cmd_reset();
    cmd_recalibrate();
}

/// Sends a sense interrupt command to the drive.
///
/// Returns st0.
static uint8_t cmd_sense_interrupt(void) {
    uint8_t result[2];
    const int err = send_command(FLOPPY_COMMAND_SENSE_INTERRUPT, NULL, 0, result, 2, 5);
    if (err != 0) {
        vga_fatalf("could not read floppy: failed to send sense interrupt (error code: %d)", err);
    }
    return result[0];
}

static void cmd_seek(size_t lba) {
    const uint8_t c = lba_2_cylinder(lba);
    const uint8_t h = lba_2_head(lba);
    const uint8_t params[2] = {(h << 2) | FLOPPY_DRIVE_NUM, c};
    const int err = send_command(FLOPPY_COMMAND_SEEK, params, 2, NULL, 0, 5);
    if (err != 0) {
        vga_fatalf("could not read floppy: failed to submit seek command (error code: %d)", err);
    }
    // Head movement is done once drive is inactive
    wait_drive_inactive();
    // Check for errors
    const uint8_t st0 = cmd_sense_interrupt();
    if ((st0 & 0xC0) > 0) {
        vga_fatalf("could not read floppy: seek command reported failure");
    }
}

static void read_sector(size_t lba, uint8_t* dest) {
    // Seek to correct cylinder
    cmd_seek(lba);
    // TODO: Issue read command
    // TODO: Poll until done
    // TODO: Check for errors and retry
    (void)dest;
}

void floppy_read(size_t lba_start_sector_idx, size_t num_sectors, uint8_t* dest) {
    init();
    for (size_t i = 0; i < num_sectors; i++) {
        read_sector(lba_start_sector_idx + i, dest);
        dest += FLOPPY_SECTOR_SIZE;
    }
    // TODO: Use
    (void)lba_2_sector;
}
