/**
 * @file mqtt_telemetry.h
 * @brief MQTT & ThingSpeak Telemetry Serialization Header
 * @author Amit Kumar (NIT Patna - Electrical Engineering)
 */

#ifndef MQTT_TELEMETRY_H
#define MQTT_TELEMETRY_H

#include <stddef.h>
#include "energy_meter_types.h"

// Format telemetry into standard JSON string for MQTT topic
size_t mqtt_format_json_payload(const grid_telemetry_t *telem, const relay_controller_t *relay, uint32_t fault_mask, char *out_buffer, size_t max_len);

// Format telemetry into ThingSpeak REST format (field1..field6)
size_t mqtt_format_thingspeak_payload(const grid_telemetry_t *telem, char *out_buffer, size_t max_len);

#endif // MQTT_TELEMETRY_H
