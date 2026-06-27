# Requirement Check Report

## Summary

- PASS: 12
- MANUAL: 9
- PLANNED: 0
- FAIL: 0
- UNKNOWN_REFERENCES: 0

## Requirement Results

| Requirement | Result | Doc Status | Evidence |
|---|---|---|---|
| FSW-REQ-001 | MANUAL | Verified | `mode_manager_tests`, `astra_fsw` output |
| FSW-REQ-002 | PASS | Verified | basic_command_fault, hil_smoke_test |
| FSW-REQ-003 | PASS | Verified | invalid_transition_rejected |
| FSW-REQ-004 | PASS | Verified | basic_command_fault, hil_smoke_test |
| FSW-REQ-005 | PASS | Verified | basic_command_fault, hil_smoke_test |
| FSW-REQ-006 | PASS | Verified | sensor_timeout_safe_mode |
| FSW-REQ-007 | PASS | Verified | watchdog_timeout_safe_mode |
| FSW-REQ-101 | MANUAL | Verified | `telemetry_packet_tests` |
| FSW-REQ-102 | MANUAL | Verified | `telemetry_packet_tests` |
| FSW-REQ-103 | MANUAL | Verified | `udp_telemetry_sender_tests`, `reports/live_telemetry_demo_output.txt` |
| FSW-REQ-104 | MANUAL | Verified | `command_packet_tests` |
| FSW-REQ-105 | MANUAL | Verified | `udp_command_receiver_tests`, `reports/command_telemetry_demo_output.txt` |
| FSW-REQ-106 | PASS | Verified | basic_command_fault, hil_smoke_test, invalid_transition_rejected, sensor_timeout_safe_mode, watchdog_timeout_safe_mode |
| VER-REQ-001 | MANUAL | Verified | `ci/run_local_tests.sh` |
| VER-REQ-002 | PASS | Verified | basic_command_fault, hil_smoke_test, invalid_transition_rejected, sensor_timeout_safe_mode, watchdog_timeout_safe_mode |
| VER-REQ-003 | MANUAL | Verified | `tools/run_all_scenarios.py` |
| VER-REQ-004 | PASS | Verified | basic_command_fault, hil_smoke_test, invalid_transition_rejected, sensor_timeout_safe_mode, watchdog_timeout_safe_mode |
| VER-REQ-005 | MANUAL | Verified | Verified |
| VER-REQ-006 | PASS | Verified | `tools/run_monte_carlo.py`, `reports/monte_carlo_report.md` |
| VER-REQ-007 | PASS | Verified | `tools/package_pi_deployment.sh`, `reports/pi_deployment_package_report.md` |
| VER-REQ-008 | PASS | Verified | hil_smoke_test |

## Scenario Evidence

| Scenario | Result | Requirements | Report |
|---|---|---|---|
| basic_command_fault | PASS | FSW-REQ-002, FSW-REQ-004, FSW-REQ-005, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_basic_command_fault_output.txt` |
| hil_smoke_test | PASS | FSW-REQ-002, FSW-REQ-004, FSW-REQ-005, FSW-REQ-106, VER-REQ-002, VER-REQ-004, VER-REQ-008 | `reports/scenario_hil_smoke_test_output.txt` |
| invalid_transition_rejected | PASS | FSW-REQ-003, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_invalid_transition_rejected_output.txt` |
| sensor_timeout_safe_mode | PASS | FSW-REQ-006, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_sensor_timeout_safe_mode_output.txt` |
| watchdog_timeout_safe_mode | PASS | FSW-REQ-007, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_watchdog_timeout_safe_mode_output.txt` |
