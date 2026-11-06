/**
 * @file grid_protection.c
 * @brief Automated Threshold Alert & Relay Isolation Logic Implementation
 * @author Amit Kumar (NIT Patna - Electrical Engineering)
 */

#include "grid_protection.h"

void grid_protection_init(relay_controller_t *relay) {
    relay->state = RELAY_STATE_CLOSED_NORMAL;
    relay->trip_count = 0;
    relay->last_trip_timestamp_ms = 0;
    relay->last_fault_reason = "NORMAL_OPERATION";
}

void grid_protection_manual_reset(relay_controller_t *relay) {
    relay->state = RELAY_STATE_CLOSED_NORMAL;
    relay->last_fault_reason = "MANUAL_RESET_CLEARED";
}

uint32_t grid_protection_evaluate(const grid_telemetry_t *telemetry, relay_controller_t *relay, uint32_t current_time_ms) {
    uint32_t active_faults = GRID_FAULT_NONE;

    // 1. Over-Current Trip Check
    if (telemetry->current_rms_a >= TRIP_OVER_CURRENT_A) {
        active_faults |= GRID_FAULT_OVER_CURRENT;
        if (relay->state == RELAY_STATE_CLOSED_NORMAL) {
            relay->state = RELAY_STATE_OPEN_ISOLATED;
            relay->trip_count++;
            relay->last_trip_timestamp_ms = current_time_ms;
            relay->last_fault_reason = "CRITICAL_OVER_CURRENT_TRIP";
        }
    }

    // 2. Over-Voltage Trip Check (V > 265 V)
    if (telemetry->voltage_rms_v >= TRIP_OVER_VOLTAGE_V) {
        active_faults |= GRID_FAULT_OVER_VOLTAGE;
        if (relay->state == RELAY_STATE_CLOSED_NORMAL) {
            relay->state = RELAY_STATE_OPEN_ISOLATED;
            relay->trip_count++;
            relay->last_trip_timestamp_ms = current_time_ms;
            relay->last_fault_reason = "GRID_OVER_VOLTAGE_SURGE";
        }
    }

    // 3. Under-Voltage Trip Check (V < 185 V)
    if (telemetry->voltage_rms_v <= TRIP_UNDER_VOLTAGE_V) {
        active_faults |= GRID_FAULT_UNDER_VOLTAGE;
        if (relay->state == RELAY_STATE_CLOSED_NORMAL) {
            relay->state = RELAY_STATE_OPEN_ISOLATED;
            relay->trip_count++;
            relay->last_trip_timestamp_ms = current_time_ms;
            relay->last_fault_reason = "GRID_BROWNOUT_UNDER_VOLTAGE";
        }
    }

    // 4. Power Factor Warning (< 0.82)
    if (telemetry->current_rms_a > 1.0f && telemetry->power_factor < WARNING_POOR_PF_LIMIT) {
        active_faults |= GRID_FAULT_POOR_PF;
    }

    // Auto-recovery hysteresis for Voltage if over-current has not tripped
    if (relay->state == RELAY_STATE_OPEN_ISOLATED) {
        bool voltage_ok = (telemetry->voltage_rms_v < RELEASE_OVER_VOLTAGE_V) && 
                          (telemetry->voltage_rms_v > RELEASE_UNDER_VOLTAGE_V);
        bool current_safe = (telemetry->current_rms_a < (TRIP_OVER_CURRENT_A * 0.8f));
        
        // Auto-reclose after 5000 ms if only caused by voltage transient
        if (voltage_ok && current_safe && (current_time_ms - relay->last_trip_timestamp_ms >= 5000)) {
            if (!(active_faults & GRID_FAULT_OVER_CURRENT)) {
                relay->state = RELAY_STATE_CLOSED_NORMAL;
                relay->last_fault_reason = "AUTO_RECLOSE_GRID_HEALTHY";
            }
        }
    }

    return active_faults;
}
