/**
 * @file energy_meter_types.h
 * @brief Data Structures, Grid Metrics & Relay Status for Smart Energy Meter
 * @author Amit Kumar (NIT Patna - Electrical Engineering)
 */

#ifndef ENERGY_METER_TYPES_H
#define ENERGY_METER_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Grid Electrical Telemetry (Measured via PZEM-004T v3.0)
typedef struct {
    float voltage_rms_v;      // Grid RMS Voltage (0.1V resolution, e.g. 230.4 V)
    float current_rms_a;      // Line Current (0.001A resolution, e.g. 4.821 A)
    float active_power_w;     // Real Power P = V * I * cos(phi) (0.1W resolution)
    float apparent_power_va;  // Apparent Power S = V * I (VA)
    float reactive_power_var; // Reactive Power Q = sqrt(S^2 - P^2) (VAR)
    float energy_kwh;         // Cumulative Energy Consumption (0.001 kWh)
    float frequency_hz;       // Line Frequency (0.1 Hz resolution, e.g. 50.0 Hz)
    float power_factor;       // cos(phi) (0.01 resolution, 0.00 to 1.00)
    uint32_t alarm_status;    // Hardware PZEM alarm flags (0xFFFF if active)
} grid_telemetry_t;

// Fault & Protection Flags
typedef enum {
    GRID_FAULT_NONE           = 0x0000,
    GRID_FAULT_OVER_CURRENT   = 0x0001, // Current exceeds trip threshold
    GRID_FAULT_OVER_VOLTAGE   = 0x0002, // Voltage > 265 V RMS
    GRID_FAULT_UNDER_VOLTAGE  = 0x0004, // Voltage < 185 V RMS
    GRID_FAULT_POOR_PF        = 0x0008, // Power factor < 0.80 warning
    GRID_FAULT_PHASE_IMBAL    = 0x0010, // Phase imbalance detected
    GRID_FAULT_SENSOR_TIMEOUT = 0x0020  // UART Modbus timeout
} grid_fault_flags_t;

// Operational Thresholds
#define GRID_NOMINAL_VOLTAGE_V    230.0f
#define GRID_NOMINAL_FREQ_HZ      50.0f
#define TRIP_OVER_VOLTAGE_V       265.0f
#define RELEASE_OVER_VOLTAGE_V    255.0f
#define TRIP_UNDER_VOLTAGE_V      185.0f
#define RELEASE_UNDER_VOLTAGE_V   195.0f
#define TRIP_OVER_CURRENT_A       16.0f   // Rated industrial load limit
#define WARNING_POOR_PF_LIMIT     0.82f   // Reactive penalty warning

// Relay Actuation State
typedef enum {
    RELAY_STATE_OPEN_ISOLATED = 0, // Load disconnected (Fault condition)
    RELAY_STATE_CLOSED_NORMAL = 1  // Load energized (Healthy grid)
} relay_state_t;

typedef struct {
    relay_state_t state;
    uint32_t trip_count;
    uint32_t last_trip_timestamp_ms;
    const char *last_fault_reason;
} relay_controller_t;

#endif // ENERGY_METER_TYPES_H
