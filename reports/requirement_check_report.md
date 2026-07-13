# Requirement Check Report

## Summary

- PASS: 9
- MANUAL: 37
- PLANNED: 15
- HISTORICAL: 1
- FAIL: 0
- TRACEABILITY_PROBLEMS: 0

A MANUAL result means the requirement has a matrix allocation but was not automatically dispositioned by the current scenario-report logic. It is not equivalent to PASS.

## Requirement Results

| Requirement | Result | Doc Status | Component | Verification Case or Evidence |
|---|---|---|---|---|
| FSW-REQ-001 | MANUAL | Verified | ModeManager | mode_manager_tests;flight_software_app_tests |
| FSW-REQ-002 | PASS | Verified | FlightSoftwareApp | basic_command_fault, hil_smoke_test |
| FSW-REQ-003 | PASS | Verified | ModeManager | invalid_transition_rejected |
| FSW-REQ-004 | PASS | Verified | ModeManager;CommandProcessor | basic_command_fault, hil_smoke_test |
| FSW-REQ-005 | PASS | Verified | CommandProcessor | basic_command_fault, hil_smoke_test |
| FSW-REQ-006 | PASS | Verified | HealthMonitor;FlightSoftwareApp | sensor_timeout_safe_mode |
| FSW-REQ-007 | PASS | Verified | Watchdog;FlightSoftwareApp | watchdog_timeout_safe_mode |
| FSW-REQ-008 | MANUAL | Implemented | RateGroupScheduler | rate_group_scheduler_tests |
| FSW-REQ-009 | MANUAL | Implemented | RateGroupScheduler | rate_group_scheduler_tests |
| FSW-REQ-010 | MANUAL | Implemented | RateGroupScheduler | rate_group_scheduler_tests |
| FSW-REQ-011 | MANUAL | Implemented | RateGroupScheduler | rate_group_scheduler_tests |
| FSW-REQ-012 | PLANNED | Planned | ModeManager | planned_mode_tests |
| FSW-REQ-013 | PLANNED | Planned | FlightSoftwareApp;RateGroupScheduler | planned_scheduler_integration |
| FSW-REQ-101 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-102 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-103 | MANUAL | Implemented | CommandPacket | command_packet_tests;controlled_crc_mutation |
| FSW-REQ-104 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-105 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-106 | PASS | Verified | CommandPacket | basic_command_fault, hil_smoke_test, invalid_transition_rejected, sensor_timeout_safe_mode, watchdog_timeout_safe_mode |
| FSW-REQ-107 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-108 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-109 | MANUAL | Implemented | FlightSoftwareApp | flight_software_app_tests |
| FSW-REQ-110 | MANUAL | Implemented | FlightSoftwareApp | flight_software_app_tests |
| FSW-REQ-111 | MANUAL | Implemented | FlightSoftwareApp | flight_software_app_tests |
| FSW-REQ-112 | MANUAL | Implemented | FlightSoftwareApp | flight_software_app_tests |
| FSW-REQ-113 | MANUAL | Verified | TelemetryPacket;FlightSoftwareApp | telemetry_packet_tests;flight_software_app_tests |
| FSW-REQ-114 | MANUAL | Verified | TelemetryPacket | telemetry_packet_tests |
| FSW-REQ-115 | PLANNED | Planned | TimeService;FlightSoftwareApp | planned_command_freshness |
| FSW-REQ-116 | PLANNED | Planned | CommandAuthorization | planned_command_authorization |
| FSW-REQ-201 | MANUAL | Verified | HealthMonitor | health_monitor_tests;flight_software_app_tests |
| FSW-REQ-202 | MANUAL | Verified | HealthMonitor | health_monitor_tests |
| FSW-REQ-203 | MANUAL | Verified | HealthMonitor | health_monitor_tests;sensor_timeout_safe_mode |
| FSW-REQ-204 | MANUAL | Verified | HealthMonitor | health_monitor_tests |
| FSW-REQ-205 | MANUAL | Verified | Watchdog | watchdog_tests |
| FSW-REQ-206 | MANUAL | Implemented | FlightSoftwareApp | flight_software_app_tests |
| FSW-REQ-207 | PLANNED | Planned | FDIRManager | planned_fault_disposition_matrix |
| FSW-REQ-208 | PLANNED | Planned | FDIRManager | planned_ten_fault_campaign |
| FSW-REQ-209 | PLANNED | Planned | FDIRManager | planned_simultaneous_fault_tests |
| FSW-REQ-210 | PLANNED | Planned | FDIRManager | planned_recovery_failure_scenario |
| VER-REQ-001 | MANUAL | Verified | BuildSystem | ci_run_local_tests |
| VER-REQ-002 | PASS | Verified | ScenarioRunner | basic_command_fault, hil_smoke_test, invalid_transition_rejected, sensor_timeout_safe_mode, watchdog_timeout_safe_mode |
| VER-REQ-003 | MANUAL | Verified | VerificationOrchestrator | run_all_scenarios |
| VER-REQ-004 | PASS | Verified | ScenarioRunner | basic_command_fault, hil_smoke_test, invalid_transition_rejected, sensor_timeout_safe_mode, watchdog_timeout_safe_mode |
| VER-REQ-005 | MANUAL | Verified | TraceabilityChecker | check_requirements |
| VER-REQ-006 | MANUAL | Verified | MonteCarloRunner | run_monte_carlo |
| VER-REQ-007 | MANUAL | Verified | PiPackager | package_pi_deployment |
| VER-REQ-008 | HISTORICAL | Historical | HistoricalHIL | hil_smoke_test |
| VER-REQ-009 | MANUAL | Implemented | CMake;UnitTests | normal_ci_build |
| VER-REQ-010 | MANUAL | Implemented | CMake;UnitTests | sanitizer_ci |
| VER-REQ-011 | MANUAL | Implemented | CMake;UnitTests | coverage_ci |
| VER-REQ-012 | MANUAL | Implemented | CMake;Core | clang_tidy_ci |
| VER-REQ-013 | MANUAL | Implemented | PythonTools;ShellTools | python_shell_smoke |
| VER-REQ-014 | MANUAL | Implemented | CommandPacket;Tests | controlled_crc_mutation |
| VER-REQ-015 | MANUAL | Implemented | Provenance | generate_baseline_manifest |
| VER-REQ-016 | PLANNED | Planned | Coverage | planned_module_coverage_report |
| VER-REQ-017 | PLANNED | Planned | CommandPacketFuzzer | planned_coverage_guided_fuzzer |
| VER-REQ-018 | PLANNED | Planned | PerformanceEvidence | planned_timing_jitter_soak |
| VER-REQ-101 | MANUAL | Implemented | DigitalThread | verification_matrix;check_requirements |
| VER-REQ-102 | PLANNED | Planned | DigitalThread | planned_impact_checker |
| VER-REQ-103 | PLANNED | Planned | AssuranceAssistant | planned_agent_retrieval_and_tools |
| VER-REQ-104 | PLANNED | Planned | AssuranceAssistant | planned_agent_permission_tests |
| VER-REQ-105 | PLANNED | Planned | AssuranceAssistant | planned_agent_frozen_evaluation |

## Scenario Evidence

| Scenario | Result | Requirements | Report |
|---|---|---|---|
| basic_command_fault | PASS | FSW-REQ-002, FSW-REQ-004, FSW-REQ-005, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_basic_command_fault_output.txt` |
| hil_smoke_test | PASS | FSW-REQ-002, FSW-REQ-004, FSW-REQ-005, FSW-REQ-106, VER-REQ-002, VER-REQ-004, VER-REQ-008 | `reports/scenario_hil_smoke_test_output.txt` |
| invalid_transition_rejected | PASS | FSW-REQ-003, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_invalid_transition_rejected_output.txt` |
| sensor_timeout_safe_mode | PASS | FSW-REQ-006, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_sensor_timeout_safe_mode_output.txt` |
| watchdog_timeout_safe_mode | PASS | FSW-REQ-007, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_watchdog_timeout_safe_mode_output.txt` |
