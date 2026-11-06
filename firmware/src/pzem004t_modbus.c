/**
 * @file pzem004t_modbus.c
 * @brief PZEM-004T v3.0 Modbus-RTU Driver Implementation
 * @author Amit Kumar (NIT Patna - Electrical Engineering)
 */

#include "pzem004t_modbus.h"
#include <math.h>

uint16_t pzem_calc_crc16(const uint8_t *buffer, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= buffer[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

size_t pzem_build_read_query(uint8_t slave_addr, uint8_t *out_buffer) {
    out_buffer[0] = slave_addr; // Slave Address (Default 0x01)
    out_buffer[1] = 0x04;       // Function Code: Read Input Registers
    out_buffer[2] = 0x00;       // Register Start High
    out_buffer[3] = 0x00;       // Register Start Low
    out_buffer[4] = 0x00;       // Number of Registers High
    out_buffer[5] = 0x0A;       // Number of Registers Low (10 registers = 20 bytes)
    
    uint16_t crc = pzem_calc_crc16(out_buffer, 6);
    out_buffer[6] = (uint8_t)(crc & 0xFF);        // CRC Low Byte
    out_buffer[7] = (uint8_t)((crc >> 8) & 0xFF); // CRC High Byte
    return 8;
}

bool pzem_parse_response(const uint8_t *frame, size_t length, grid_telemetry_t *telemetry) {
    if (frame == NULL || telemetry == NULL) return false;
    if (length < 25) return false; // Expected 25 bytes

    // 1. Validate CRC
    uint16_t expected_crc = ((uint16_t)frame[24] << 8) | frame[23];
    uint16_t computed_crc = pzem_calc_crc16(frame, 23);
    if (expected_crc != computed_crc) {
        return false; // CRC error
    }

    // 2. Validate Function Code and Byte Count
    if (frame[1] != 0x04 || frame[2] != 0x14) {
        return false;
    }

    // 3. Unpack Registers (Big Endian)
    // Voltage: Reg 0x0000 (Bytes 3-4), 0.1V
    uint16_t raw_v = ((uint16_t)frame[3] << 8) | frame[4];
    telemetry->voltage_rms_v = raw_v * 0.1f;

    // Current: Reg 0x0001 - 0x0002 (Bytes 5-8), 0.001A
    // PZEM ordering: Low 16-bits in Bytes 5-6, High 16-bits in Bytes 7-8
    uint32_t raw_i = ((uint32_t)frame[7] << 24) | ((uint32_t)frame[8] << 16) |
                     ((uint32_t)frame[5] << 8)  | (uint32_t)frame[6];
    telemetry->current_rms_a = raw_i * 0.001f;

    // Active Power: Reg 0x0003 - 0x0004 (Bytes 9-12), 0.1W
    uint32_t raw_p = ((uint32_t)frame[11] << 24) | ((uint32_t)frame[12] << 16) |
                     ((uint32_t)frame[9] << 8)   | (uint32_t)frame[10];
    telemetry->active_power_w = raw_p * 0.1f;

    // Energy: Reg 0x0005 - 0x0006 (Bytes 13-16), 1Wh -> convert to kWh
    uint32_t raw_e = ((uint32_t)frame[15] << 24) | ((uint32_t)frame[16] << 16) |
                     ((uint32_t)frame[13] << 8)  | (uint32_t)frame[14];
    telemetry->energy_kwh = raw_e * 0.001f;

    // Frequency: Reg 0x0007 (Bytes 17-18), 0.1Hz
    uint16_t raw_f = ((uint16_t)frame[17] << 8) | frame[18];
    telemetry->frequency_hz = raw_f * 0.1f;

    // Power Factor: Reg 0x0008 (Bytes 19-20), 0.01
    uint16_t raw_pf = ((uint16_t)frame[19] << 8) | frame[20];
    telemetry->power_factor = raw_pf * 0.01f;

    // Alarm Status: Reg 0x0009 (Bytes 21-22)
    telemetry->alarm_status = ((uint16_t)frame[21] << 8) | frame[22];

    // Compute Derived Power Quality Metrics
    telemetry->apparent_power_va = telemetry->voltage_rms_v * telemetry->current_rms_a;
    float s_sq = telemetry->apparent_power_va * telemetry->apparent_power_va;
    float p_sq = telemetry->active_power_w * telemetry->active_power_w;
    telemetry->reactive_power_var = (s_sq > p_sq) ? sqrtf(s_sq - p_sq) : 0.0f;

    return true;
}
