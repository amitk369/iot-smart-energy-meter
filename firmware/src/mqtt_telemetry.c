/**
 * @file mqtt_telemetry.c
 * @brief MQTT & ThingSpeak Telemetry Serialization Implementation
 * @author Amit Kumar (NIT Patna - Electrical Engineering)
 */

#include "mqtt_telemetry.h"
#include <stdio.h>

size_t mqtt_format_json_payload(const grid_telemetry_t *telem, const relay_controller_t *relay, uint32_t fault_mask, char *out_buffer, size_t max_len) {
    if (telem == NULL || out_buffer == NULL || max_len == 0) return 0;

    int written = snprintf(out_buffer, max_len,
        "{"
        "\"device_id\":\"NITP-SMART-METER-01\","
        "\"voltage_rms\":%.1f,"
        "\"current_rms\":%.3f,"
        "\"active_power_w\":%.1f,"
        "\"reactive_power_var\":%.1f,"
        "\"apparent_power_va\":%.1f,"
        "\"power_factor\":%.2f,"
        "\"frequency_hz\":%.1f,"
        "\"energy_kwh\":%.3f,"
        "\"relay_state\":\"%s\","
        "\"fault_mask\":%u,"
        "\"fault_reason\":\"%s\""
        "}",
        telem->voltage_rms_v,
        telem->current_rms_a,
        telem->active_power_w,
        telem->reactive_power_var,
        telem->apparent_power_va,
        telem->power_factor,
        telem->frequency_hz,
        telem->energy_kwh,
        (relay->state == RELAY_STATE_CLOSED_NORMAL) ? "CLOSED" : "ISOLATED",
        fault_mask,
        relay->last_fault_reason
    );

    return (written > 0 && (size_t)written < max_len) ? (size_t)written : 0;
}

size_t mqtt_format_thingspeak_payload(const grid_telemetry_t *telem, char *out_buffer, size_t max_len) {
    if (telem == NULL || out_buffer == NULL || max_len == 0) return 0;

    int written = snprintf(out_buffer, max_len,
        "field1=%.1f&field2=%.3f&field3=%.1f&field4=%.2f&field5=%.1f&field6=%.3f",
        telem->voltage_rms_v,
        telem->current_rms_a,
        telem->active_power_w,
        telem->power_factor,
        telem->frequency_hz,
        telem->energy_kwh
    );

    return (written > 0 && (size_t)written < max_len) ? (size_t)written : 0;
}
