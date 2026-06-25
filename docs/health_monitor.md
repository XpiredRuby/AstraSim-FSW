# HealthMonitor

`HealthMonitor` evaluates basic flight-software health inputs and converts them into health status and fault codes.

## Purpose

The health monitor is the first step toward autonomous fault detection.

Instead of injecting faults manually forever, future versions of the app can evaluate real measurements and automatically generate faults.

## Inputs

`HealthSnapshot` contains:

- CPU load percent
- Memory load percent
- Sensor data age
- Payload heartbeat age
- Flight software loop duration

## Outputs

`HealthReport` contains:

- Overall health status
- Fault code
- Human-readable message
- Individual health flags for CPU, memory, sensor, payload, and loop timing

## Current Fault Mapping

| Condition | Fault |
|---|---|
| CPU warning / critical threshold exceeded | `CPU_OVERLOAD` |
| Memory warning / critical threshold exceeded | `MEMORY_OVERLOAD` |
| Sensor timeout exceeded | `SENSOR_TIMEOUT` |
| Payload heartbeat timeout exceeded | `PAYLOAD_HEARTBEAT_TIMEOUT` |
| Loop deadline missed | `WATCHDOG_DEADLINE_MISS` |

## Current Severity Rules

- Nominal values produce `OK`.
- CPU, memory, and payload threshold warnings can produce `WARNING`.
- Critical CPU, critical memory, sensor timeout, and loop deadline miss produce `CRITICAL`.
- Critical faults override warning faults in the final reported status.
- Individual boolean flags still record all failed checks.

## Future Use

Future versions should connect `HealthMonitor` into `FlightSoftwareApp` so health reports can automatically trigger `CommandProcessor` / `ModeManager` fault handling.
