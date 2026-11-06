/**
 * @file test_energy_meter.c
 * @brief Unit Tests for PZEM Modbus-RTU, Grid Protection & MQTT JSON Formatter
 * @author Amit Kumar (NIT Patna - Electrical Engineering)
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "../firmware/include/pzem004t_modbus.h"
#include "../firmware/include/grid_protection.h"
#include "../firmware/include/mqtt_telemetry.h"

void test_modbus_crc16() {
    printf("[TEST] Testing Modbus-RTU CRC-16 Calculation...\n");
    // Standard query frame: 0x01, 0x04, 0x00, 0x00, 0x00, 0x0A -> expected CRC: 0x70, 0x0D
    uint8_t query[8];
    size_t len = pzem_build_read_query(0x01, query);
    assert(len == 8);
    assert(query[0] == 0x01);
    assert(query[1] == 0x04);
    assert(query[6] == 0x70); // CRC low byte
    assert(query[7] == 0x0D); // CRC high byte
    printf("  -> Modbus CRC-16 generation PASSED.\n");
}

void test_pzem_response_parsing() {
    printf("[TEST] Testing PZEM Response Decoding & Power Quality Math...\n");
    // Synthetic valid 25-byte frame with 230.5V, 5.000A, 1152.5W, 50.0Hz, PF=1.00
    uint8_t frame[25] = {
        0x01, 0x04, 0x14,
        0x09, 0x01,             // Voltage = 2305 (230.5V)
        0x13, 0x88, 0x00, 0x00, // Current = 5000 (5.000A)
        0x2D, 0x05, 0x00, 0x00, // Active Power = 11525 (1152.5W)
        0x03, 0xE8, 0x00, 0x00, // Energy = 1000 (1.000 kWh)
        0x01, 0xF4,             // Frequency = 500 (50.0 Hz)
        0x00, 0x64,             // Power Factor = 100 (1.00)
        0x00, 0x00,             // Alarm = 0
        0x00, 0x00              // CRC will be filled below
    };
    uint16_t crc = pzem_calc_crc16(frame, 23);
    frame[23] = (uint8_t)(crc & 0xFF);
    frame[24] = (uint8_t)((crc >> 8) & 0xFF);

    grid_telemetry_t telem;
    bool ok = pzem_parse_response(frame, 25, &telem);
    assert(ok == true);
    assert(fabsf(telem.voltage_rms_v - 230.5f) < 0.1f);
    assert(fabsf(telem.current_rms_a - 5.000f) < 0.005f);
    assert(fabsf(telem.active_power_w - 1152.5f) < 0.5f);
    assert(fabsf(telem.power_factor - 1.00f) < 0.01f);
    assert(fabsf(telem.frequency_hz - 50.0f) < 0.1f);
    printf("  -> PZEM telemetry parsing PASSED.\n");
}

void test_automated_relay_isolation() {
    printf("[TEST] Testing Automated Relay Isolation Under Over-Current...\n");
    relay_controller_t relay;
    grid_protection_init(&relay);
    assert(relay.state == RELAY_STATE_CLOSED_NORMAL);

    grid_telemetry_t telem = {
        .voltage_rms_v = 230.0f,
        .current_rms_a = 18.2f, // Above 16.0A threshold
        .active_power_w = 4000.0f,
        .power_factor = 0.95f
    };

    uint32_t faults = grid_protection_evaluate(&telem, &relay, 1000);
    assert(faults & GRID_FAULT_OVER_CURRENT);
    assert(relay.state == RELAY_STATE_OPEN_ISOLATED);
    assert(relay.trip_count == 1);
    printf("  -> Relay instant trip PASSED.\n");
}

void test_mqtt_json_formatter() {
    printf("[TEST] Testing MQTT 1-Second Telemetry JSON Serialization...\n");
    grid_telemetry_t telem = {
        .voltage_rms_v = 230.2f,
        .current_rms_a = 4.350f,
        .active_power_w = 952.1f,
        .apparent_power_va = 1001.4f,
        .reactive_power_var = 309.5f,
        .energy_kwh = 12.450f,
        .frequency_hz = 50.0f,
        .power_factor = 0.95f
    };
    relay_controller_t relay;
    grid_protection_init(&relay);

    char buf[512];
    size_t len = mqtt_format_json_payload(&telem, &relay, GRID_FAULT_NONE, buf, sizeof(buf));
    assert(len > 0);
    assert(strstr(buf, "\"voltage_rms\":230.2") != NULL);
    assert(strstr(buf, "\"relay_state\":\"CLOSED\"") != NULL);
    printf("  -> MQTT JSON payload PASSED.\n");
}

int main() {
    printf("==================================================\n");
    printf(" Running Smart Energy Meter C Unit Test Harness   \n");
    printf("==================================================\n");
    test_modbus_crc16();
    test_pzem_response_parsing();
    test_automated_relay_isolation();
    test_mqtt_json_formatter();
    printf("==================================================\n");
    printf(" ALL ENERGY METER UNIT TESTS PASSED SUCCESSFULLY! \n");
    printf("==================================================\n");
    return 0;
}
