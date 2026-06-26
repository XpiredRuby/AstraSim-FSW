# Requirements

AstraSim-FSW requirements are written as testable flight-software and verification requirements.

Each requirement should map to at least one verification method:

- Unit test
- YAML scenario
- Demo evidence report
- Future Raspberry Pi / HIL test

## Flight Software Functional Requirements

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| FSW-REQ-001 | Flight software shall start in BOOT mode. | Unit test / startup demo | `mode_manager_tests`, `astra_fsw` output | Verified |
| FSW-REQ-002 | Flight software shall transition from BOOT to NOMINAL on a valid SET_MODE command. | YAML scenario | `scenarios/basic_command_fault.yaml` | Verified |
| FSW-REQ-003 | Flight software shall reject invalid mode transitions. | Unit test / YAML scenario | `mode_manager_tests`, `scenarios/invalid_transition_rejected.yaml` | Verified |
| FSW-REQ-004 | Flight software shall transition to DEGRADED_PAYLOAD when CPU_OVERLOAD is injected in NOMINAL mode. | Unit test / YAML scenario | `command_processor_tests`, `scenarios/basic_command_fault.yaml` | Verified |
| FSW-REQ-005 | Flight software shall clear the active fault when a CLEAR_FAULT command is accepted. | Unit test / YAML scenario | `command_processor_tests`, `scenarios/basic_command_fault.yaml` | Verified |
| FSW-REQ-006 | Flight software shall enter SAFE mode when the health monitor detects a sensor timeout. | Unit test / YAML scenario | `health_monitor_tests`, `scenarios/sensor_timeout_safe_mode.yaml` | Verified |
| FSW-REQ-007 | Flight software shall enter SAFE mode when the watchdog detects a missed deadline or timeout. | Unit test / YAML scenario | `watchdog_tests`, `scenarios/watchdog_timeout_safe_mode.yaml` | Verified |

## Command and Telemetry Requirements

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| FSW-REQ-101 | Flight software shall serialize and deserialize telemetry packets with CRC validation. | Unit test | `telemetry_packet_tests` | Verified |
| FSW-REQ-102 | Flight software shall reject telemetry packets with invalid CRC or invalid fields. | Unit test | `telemetry_packet_tests` | Verified |
| FSW-REQ-103 | Flight software shall transmit telemetry over UDP. | Unit test / demo | `udp_telemetry_sender_tests`, `reports/live_telemetry_demo_output.txt` | Verified |
| FSW-REQ-104 | Flight software shall serialize and deserialize command packets with CRC validation. | Unit test | `command_packet_tests` | Verified |
| FSW-REQ-105 | Flight software shall receive command packets over UDP. | Unit test / demo | `udp_command_receiver_tests`, `reports/command_telemetry_demo_output.txt` | Verified |
| FSW-REQ-106 | Telemetry shall include acknowledgement fields for the last processed ground command. | Unit test / YAML scenario | `telemetry_packet_tests`, scenario reports | Verified |

## Verification Framework Requirements

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| VER-REQ-001 | The project shall provide a repeatable local build and unit-test command. | Script execution | `ci/run_local_tests.sh` | Verified |
| VER-REQ-002 | The project shall provide YAML scenario tests for command/telemetry behavior. | Tool execution | `tools/run_scenario.py` | Verified |
| VER-REQ-003 | The project shall provide one command to run all YAML scenarios. | Tool execution | `tools/run_all_scenarios.py` | Verified |
| VER-REQ-004 | The project shall generate evidence reports for scenario runs. | Report inspection | `reports/scenario_*_output.txt` | Verified |
| VER-REQ-005 | The project shall provide an automated requirement checker that maps requirements to evidence. | Future tool | Planned | Planned |
| VER-REQ-006 | The project shall provide a Monte Carlo regression runner. | Future tool | Planned | Planned |
| VER-REQ-007 | The project shall provide Raspberry Pi deployment evidence. | Future HIL/SIL report | Planned | Planned |
| VER-REQ-008 | The project shall provide a HIL test procedure and final HIL report. | Future HIL report | Planned | Planned |

## Current Verification Snapshot

Current verified requirement count:

```text
Verified: 17
Planned: 4
Total: 21
```

Current automated evidence:

```text
Unit tests: 9/9 passing
YAML scenarios: 4/4 passing
Full suite command: python3 tools/run_all_scenarios.py
```
