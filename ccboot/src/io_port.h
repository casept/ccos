#pragma once

#include <stdint.h>

uint8_t port_read_u8(uint16_t port);
void port_write_u8(uint16_t port, uint8_t value);
