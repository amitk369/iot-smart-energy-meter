/**
 * @file grid_protection.h
 * @brief Automated Threshold Alert & Relay Isolation Logic Header
 * @author Amit Kumar (NIT Patna - Electrical Engineering)
 */

#ifndef GRID_PROTECTION_H
#define GRID_PROTECTION_H

#include "energy_meter_types.h"

void grid_protection_init(relay_controller_t *relay);
uint32_t grid_protection_evaluate(const grid_telemetry_t *telemetry, relay_controller_t *relay, uint32_t current_time_ms);
void grid_protection_manual_reset(relay_controller_t *relay);

#endif // GRID_PROTECTION_H
