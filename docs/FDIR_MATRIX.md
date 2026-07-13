# ASTRA-OS FDIR Disposition Matrix

## Scope

This matrix distinguishes a **policy disposition** from an **implemented detector** and from **end-to-end verification**. The presence of a `FaultCode` or tested response does not prove that the corresponding real-world failure can currently be detected, isolated, or recovered on hardware.

| Fault | Detection source | Detector status | Severity | Priority | Primary response | Latching intent | Recovery rule | Current verification status |
|---|---|---|---|---:|---|---|---|---|
| `SENSOR_TIMEOUT` | Health monitor sensor age | Implemented in SIL | Critical | 90 | Enter SAFE | Latched | Operator clear after valid-sensor dwell | Unit and deterministic scenario |
| `SENSOR_INVALID_DATA` | Sensor validity monitor | Policy only | Critical | 85 | Enter SAFE | Latched | Operator clear after valid-sensor dwell | Disposition/unit test only |
| `PAYLOAD_HEARTBEAT_TIMEOUT` | Health monitor payload age | Implemented in SIL | Degraded | 50 | Enter DEGRADED_PAYLOAD | Latched | Clear after heartbeat-recovery dwell | Health unit test; full recovery dwell not implemented |
| `CPU_OVERLOAD` | Resource monitor | Implemented in SIL | Degraded | 60 | Enter DEGRADED_PAYLOAD | Latched | Clear after CPU-recovery dwell | Unit, scenario, simultaneous-fault, and reassertion tests |
| `MEMORY_OVERLOAD` | Resource monitor | Implemented in SIL | Critical | 80 | Enter SAFE | Latched | Operator clear after memory-recovery dwell | Health/FDIR unit tests; persistence dwell not implemented |
| `TELEMETRY_SOCKET_FAILURE` | Telemetry transport | Partial transport signaling | Critical | 75 | Enter SAFE | Latched | Operator clear after transport recovery | Disposition/unit test; end-to-end automatic path incomplete |
| `COMMAND_BAD_CRC` | Command decoder | Decoder rejects packet; event-to-FDIR bridge incomplete | Advisory | 20 | Hold current mode | Not latched | Automatic after valid packet | Parser, mutation, and fuzz evidence; no state-changing response |
| `COMMAND_UNKNOWN_ID` | Command decoder | Decoder rejects packet; event-to-FDIR bridge incomplete | Advisory | 15 | Hold current mode | Not latched | Automatic after valid command | Parser and fuzz evidence; no state-changing response |
| `COMMAND_TIMEOUT` | Command-link monitor | Policy only | Critical | 70 | Enter SAFE | Latched | Operator clear after link recovery | Disposition/unit test only |
| `WATCHDOG_DEADLINE_MISS` | Watchdog | Implemented in SIL | Critical | 100 | Enter SAFE | Latched | Operator clear after timing investigation | Unit, scenario, and simultaneous-fault tests |

## Deterministic simultaneous-fault policy

1. Ignore `NONE` observations.
2. Select the active fault with the highest configured priority.
3. Break equal-priority ties using the lower numeric fault code.
4. Process only the selected internal fault during the current application cycle.
5. Preserve all observed health/watchdog reports so diagnostics can show non-selected candidates.
6. Re-evaluate every cycle; unresolved conditions reassert after a ground `CLEAR_FAULT` command.
7. If a degraded response is not a valid transition from the current mode, fall back to SAFE.

The current highest-priority fault is `WATCHDOG_DEADLINE_MISS` at priority 100.

## Remaining FDIR work

- implement persistence and recovery dwell counters rather than documenting intent only;
- add an explicit command-link timeout detector;
- connect decoder rejection events to advisory fault/event telemetry without treating malformed packets as commands;
- isolate telemetry transport failure reporting from the sender implementation;
- add sensor disagreement and actuator nonresponse inputs;
- add queue-overflow, invalid-configuration, process-restart, and repeated-recovery-failure cases;
- preserve the complete active-fault set rather than only the primary fault;
- create deterministic scenario files for each independently exercised FDIR case;
- repeat applicable campaigns on Raspberry Pi with target provenance.
