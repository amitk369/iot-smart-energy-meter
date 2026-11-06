/**
 * @file pzem004t_modbus.h
 * @brief PZEM-004T v3.0 UART Modbus-RTU Protocol Driver Header
 * @author Amit Kumar (NIT Patna - Electrical Engineering)
 */

#ifndef PZEM004T_MODBUS_H
#define PZEM004T_MODBUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "energy_meter_types.h"

// Modbus RTU CRC-16 calculation
uint16_t pzem_calc_crc16(const uint8_t *buffer, size_t length);

// Construct 8-byte Read Input Registers query frame (Func 0x04, Regs 0x0000 to 0x0009)
size_t pzem_build_read_query(uint8_t slave_addr, uint8_t *out_buffer);

// Parse 25-byte raw response frame into structured telemetry metrics
bool pzem_parse_response(const uint8_t *frame, size_t length, grid_telemetry_t *telemetry);

#endif // PZEM004T_MODBUS_H
