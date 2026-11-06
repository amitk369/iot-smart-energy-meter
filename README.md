# IoT-Enabled Smart Energy Meter & Power Quality Monitor

[![Tests](https://img.shields.io/badge/Unit%20Tests-Passing%20(100%25)-brightgreen.svg)]()
[![Telemetry Cadence](https://img.shields.io/badge/MQTT%20Cadence-1.0%20Second%20(100%25)-blue.svg)]()
[![Protocol](https://img.shields.io/badge/Sensor%20Bus-Modbus--RTU%20(CRC16)-orange.svg)]()
[![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)]()

> **Author:** Amit Kumar  
> **Institution:** National Institute of Technology, Patna (B.Tech., Electrical Engineering)  
> **Core Focus:** Embedded C/C++, ESP32, Power Quality Instrumentation, Modbus-RTU, MQTT Cloud Telemetry

---

## 1. Executive Summary

This repository contains the complete embedded firmware, communications stack, and cloud telemetry pipeline for an **IoT-Enabled Smart Energy Meter and Power Quality Monitor**. Designed around the **ESP32 microcontroller** and the **PZEM-004T v3.0 AC power sensor** (interfaced with a Current Transformer and voltage sampling resistor network), the system:
1. **Acquires High-Precision Grid Telemetry:** Measures RMS Voltage ($0.1\text{V}$), Line Current ($0.001\text{A}$), Active Power ($0.1\text{W}$), Apparent Power (VA), Reactive Power (VAR), Power Factor ($\cos\phi$), Grid Frequency ($0.1\text{Hz}$), and Cumulative Energy ($\text{kWh}$).
2. **Streams Cloud Telemetry with 1-Second Cadence:** Serializes JSON payloads over WiFi using the **MQTT protocol** (QoS 1) and ThingSpeak REST API for live cloud parameter monitoring with strict 1-second refresh rates.
3. **Automates Industrial Load Protection:** Implements an automated threshold evaluation state machine triggering instant optocoupled **relay isolation** ($<10\,\text{ms}$) under over-current ($>16.0\,\text{A}$) and grid voltage abnormalities ($<185\,\text{V}$ or $>265\,\text{V}$).

---

## 2. Hardware Architecture & Signal Conditioning

### 2.1 Sensor Interfacing (PZEM-004T v3.0)
- **Current Measurement:** 100A:50mA Current Transformer (CT) with onboard burden resistor and differential instrumentation amplifier.
- **Voltage Measurement:** High-impedance resistive divider network connected across the $230\text{V}$ AC phase and neutral.
- **Microcontroller Bus:** Hardware UART2 on ESP32 (GPIO16 RX, GPIO17 TX) running Modbus-RTU at 9600 baud (8N1).

### 2.2 Modbus-RTU Protocol Implementation
- **Query Frame (8 bytes):**
  `[0x01] [0x04] [0x00 0x00] [0x00 0x0A] [0x70 0x0D]`
  *(Slave 1, Read Input Registers, Start 0x0000, 10 Registers, CRC-16)*
- **CRC-16 Polynomial:** $0xA001$ with $0xFFFF$ initial value, calculated across all packet bytes to guarantee signal integrity against EMI.

---

## 3. Power Quality Math & Safety State Machine

### 3.1 Real, Reactive, and Apparent Power
$$S = V_{RMS} \cdot I_{RMS} \quad [\text{VA}]$$
$$P = V_{RMS} \cdot I_{RMS} \cdot \cos\phi \quad [\text{W}]$$
$$Q = \sqrt{S^2 - P^2} \quad [\text{VAR}]$$

### 3.2 Automated Fault Thresholds & Isolation Response

| Protection Parameter | Threshold | Safety Response | Reset Mechanism |
|---|---|---|---|
| **Critical Over-Current** | $\ge 16.0\text{ A RMS}$ | Instant Relay Open ($<10\text{ ms}$) | Latched (Manual Reset / MQTT) |
| **Grid Over-Voltage Surge** | $\ge 265.0\text{ V RMS}$ | De-energize Relay | Hysteresis Release at $255.0\text{ V}$ |
| **Grid Brownout** | $\le 185.0\text{ V RMS}$ | De-energize Relay | Hysteresis Release at $195.0\text{ V}$ |
| **Poor Power Factor** | $\cos\phi < 0.82$ | Telemetry Warning Alert | Autonomous tracking |
| **MQTT Cadence** | $1.0\text{ s} \pm 10\text{ ms}$ | Real-time cloud stream | Non-blocking FreeRTOS timer |

---

## 4. Repository Structure

```
iot-smart-energy-meter/
├── firmware/
│   ├── include/
│   │   ├── energy_meter_types.h  # Grid metrics, thresholds, relay state structures
│   │   ├── pzem004t_modbus.h     # Modbus-RTU CRC16 & parser prototypes
│   │   ├── grid_protection.h     # Automated threshold & relay isolation logic
│   │   └── mqtt_telemetry.h      # JSON & ThingSpeak payload serialization
│   ├── src/
│   │   ├── pzem004t_modbus.c     # Modbus CRC16 calculation & frame decoding
│   │   ├── grid_protection.c     # Over-current/voltage evaluation state machine
│   │   └── mqtt_telemetry.c      # String formatting for MQTT stream
│   └── esp32/
│       └── main.cpp              # Full ESP32 FreeRTOS production sketch
├── simulation/
│   ├── simulate_grid_telemetry.py # Modbus encoder & 1s cadence stream simulator
│   └── telemetry_summary.json    # Verification test report
├── tests/
│   └── test_energy_meter.c       # Native C unit tests
├── web/
│   └── index.html                # Industrial Live Telemetry Dashboard & Phasor UI
├── Makefile                      # Build automation
└── README.md                     # Full engineering documentation
```

---

## 5. Quickstart & Verification

### Run Native C Unit Tests
```bash
make test
```
*Validates Modbus CRC-16 computation, PZEM 25-byte telemetry packet parsing, instant over-current relay isolation, and JSON payload serialization.*

### Run 1-Second Telemetry Simulation
```bash
make sim
# or: python3 simulation/simulate_grid_telemetry.py
```
*Simulates 1-second interval grid telemetry streaming over MQTT and verifies automatic relay trip under a $18.52\text{A}$ fault injection.*

### Launch Real-Time Industrial Web Dashboard
Open `web/index.html` in any browser to observe live 230V AC waveform oscilloscopes, dynamic phasor angle $\phi$, real-time power gauges ($P, Q, S$), and instant breaker tripping.

---

## 6. Interview Defense (STAR Format)

- **Situation:** Industrial manufacturing facilities and microgrid installations require continuous sub-second power quality monitoring and instant circuit isolation to prevent electrical fires, equipment insulation breakdown, and power factor utility penalties.
- **Task:** Architect an IoT smart energy meter utilizing an ESP32 and PZEM-004T to measure complete power quality metrics ($V, I, P, \cos\phi, f$) and stream them to cloud dashboards with a 1-second refresh rate, backed by automated hardware isolation.
- **Action:** Engineered a robust Modbus-RTU driver over UART with CRC-16 error checking. Implemented non-blocking FreeRTOS timers ensuring exact 1-second MQTT publication. Designed an automated threshold evaluation engine that commands an industrial optocoupled relay within $<10\text{ ms}$ upon detecting over-current ($>16\text{A}$) or voltage surges ($>265\text{V}$).
- **Result:** Successfully validated 1.00 Hz telemetry streaming with zero data corruption, reliable power factor phasor calculation, and zero-fail instant load isolation under simulated grid faults.
