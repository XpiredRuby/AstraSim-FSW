# Requirement Check Report

## Summary

- PASS: 21
- MANUAL: 46
- PLANNED: 0
- HISTORICAL: 0
- FAIL: 0
- REGISTERED_CTESTS: 20
- TRACEABILITY_PROBLEMS: 0

A MANUAL result means the requirement has a matrix allocation but was not automatically dispositioned by scenario or committed report logic. It is not equivalent to PASS.

## Requirement Results

| Requirement | Result | Doc Status | Component | Verification Case or Evidence |
|---|---|---|---|---|
| FSW-REQ-001 | MANUAL | Verified | ModeManager | mode_manager_tests;flight_software_app_tests |
| FSW-REQ-002 | PASS | Verified | FlightSoftwareApp | basic_command_fault, hil_smoke_test |
| FSW-REQ-003 | PASS | Verified | ModeManager | extended_modes, invalid_transition_rejected |
| FSW-REQ-004 | PASS | Verified | ModeManager;CommandProcessor | basic_command_fault, hil_smoke_test |
| FSW-REQ-005 | PASS | Verified | CommandProcessor | basic_command_fault, hil_smoke_test |
| FSW-REQ-006 | PASS | Verified | HealthMonitor;FlightSoftwareApp | sensor_timeout_safe_mode |
| FSW-REQ-007 | PASS | Verified | Watchdog;FlightSoftwareApp | watchdog_timeout_safe_mode |
| FSW-REQ-008 | MANUAL | Verified | RateGroupScheduler | rate_group_scheduler_tests;flight_software_executive_tests |
| FSW-REQ-009 | MANUAL | Verified | RateGroupScheduler | rate_group_scheduler_tests;flight_software_executive_tests |
| FSW-REQ-010 | MANUAL | Verified | RateGroupScheduler | rate_group_scheduler_tests |
| FSW-REQ-011 | MANUAL | Verified | RateGroupScheduler | rate_group_scheduler_tests;host_timing_campaign |
| FSW-REQ-012 | PASS | Verified | ModeManager;Protocol | extended_modes |
| FSW-REQ-013 | MANUAL | Verified | FlightSoftwareExecutive | flight_software_executive_tests;host_timing_campaign |
| FSW-REQ-101 | MANUAL | Verified | CommandPacket | command_packet_tests;udp_command_receiver_tests |
| FSW-REQ-102 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-103 | MANUAL | Verified | CommandPacket | command_packet_tests;controlled_crc_mutation |
| FSW-REQ-104 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-105 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-106 | PASS | Verified | CommandPacket;UDPCommandReceiver | basic_command_fault, extended_modes, hil_smoke_test, invalid_transition_rejected, sensor_timeout_safe_mode, watchdog_timeout_safe_mode |
| FSW-REQ-107 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-108 | MANUAL | Verified | CommandPacket | command_packet_tests |
| FSW-REQ-109 | MANUAL | Verified | GroundCommandGuard;FlightSoftwareApp | ground_command_guard_tests;flight_software_app_tests |
| FSW-REQ-110 | MANUAL | Verified | GroundCommandGuard;FlightSoftwareApp | ground_command_guard_tests;flight_software_app_tests |
| FSW-REQ-111 | MANUAL | Verified | GroundCommandGuard;FlightSoftwareApp | ground_command_guard_tests;flight_software_app_tests |
| FSW-REQ-112 | PASS | Verified | GroundCommandGuard;CommandAuthorizer;FlightSoftwareApp | command_timestamp_guard |
| FSW-REQ-113 | PASS | Verified | TelemetryPacket;UDPTelemetrySender;FlightSoftwareApp | command_timestamp_guard, recovery_failure_failsafe |
| FSW-REQ-114 | MANUAL | Verified | TelemetryPacket | telemetry_packet_tests |
| FSW-REQ-115 | PASS | Verified | GroundCommandGuard;FlightSoftwareApp | command_timestamp_guard |
| FSW-REQ-116 | MANUAL | Verified | CommandAuthorizer;FlightSoftwareApp | command_authorizer_tests;flight_software_app_tests |
| FSW-REQ-201 | MANUAL | Verified | HealthMonitor | health_monitor_tests;flight_software_app_tests |
| FSW-REQ-202 | MANUAL | Verified | HealthMonitor | health_monitor_tests |
| FSW-REQ-203 | MANUAL | Verified | HealthMonitor | health_monitor_tests;sensor_timeout_safe_mode |
| FSW-REQ-204 | MANUAL | Verified | HealthMonitor | health_monitor_tests |
| FSW-REQ-205 | MANUAL | Verified | Watchdog | watchdog_tests |
| FSW-REQ-206 | MANUAL | Verified | FlightSoftwareApp;FDIRManager | flight_software_app_tests;simultaneous_fault_tests |
| FSW-REQ-207 | PASS | Verified | FDIRManager | fdir_manager_tests;run_fdir_campaign |
| FSW-REQ-208 | PASS | Verified | FDIRManager | run_fdir_campaign;fdir_manager_tests |
| FSW-REQ-209 | PASS | Verified | FDIRManager | recovery_failure_failsafe |
| FSW-REQ-210 | PASS | Verified | RecoverySupervisor;CommandProcessor | recovery_failure_failsafe |
| FSW-REQ-211 | MANUAL | Verified | EventLogger;FlightSoftwareApp | event_logger_tests;event_integration_tests |
| FSW-REQ-212 | MANUAL | Verified | EventLogger | event_logger_tests |
| FSW-REQ-301 | MANUAL | Verified | ConfigurationService | configuration_service_tests |
| FSW-REQ-302 | MANUAL | Verified | ConfigurationService | configuration_service_tests |
| FSW-REQ-303 | MANUAL | Verified | ConfigurationService | configuration_service_tests |
| VER-REQ-001 | MANUAL | Verified | BuildSystem | ci_run_local_tests |
| VER-REQ-002 | PASS | Verified | ScenarioRunner | basic_command_fault, command_timestamp_guard, extended_modes, hil_smoke_test, invalid_transition_rejected, recovery_failure_failsafe, sensor_timeout_safe_mode, watchdog_timeout_safe_mode |
| VER-REQ-003 | MANUAL | Verified | VerificationOrchestrator | run_all_scenarios |
| VER-REQ-004 | PASS | Verified | ScenarioRunner | basic_command_fault, hil_smoke_test, invalid_transition_rejected, sensor_timeout_safe_mode, watchdog_timeout_safe_mode |
| VER-REQ-005 | MANUAL | Verified | TraceabilityChecker | check_requirements;test_traceability |
| VER-REQ-006 | MANUAL | Verified | MonteCarloRunner | run_monte_carlo |
| VER-REQ-007 | MANUAL | Verified | PiPackager | package_pi_deployment |
| VER-REQ-008 | PASS | Verified | Provenance;RaspberryPiEvidence | hil_smoke_test |
| VER-REQ-009 | MANUAL | Verified | CMake;UnitTests | normal_ci_build |
| VER-REQ-010 | MANUAL | Verified | CMake;UnitTests | sanitizer_ci |
| VER-REQ-011 | MANUAL | Verified | Coverage | coverage_ci |
| VER-REQ-012 | MANUAL | Verified | CMake;Core | clang_tidy_ci |
| VER-REQ-013 | MANUAL | Verified | PythonTools;ShellTools | python_shell_smoke;python_tool_tests |
| VER-REQ-014 | MANUAL | Verified | CommandPacket;Tests | controlled_crc_mutation |
| VER-REQ-015 | MANUAL | Verified | Provenance | generate_baseline_manifest |
| VER-REQ-016 | MANUAL | Verified | Coverage | summarize_lcov;coverage_ci |
| VER-REQ-017 | MANUAL | Verified | CommandPacketFuzzer | command_fuzz;generate_command_fuzz_corpus |
| VER-REQ-018 | MANUAL | Verified | PerformanceEvidence | host_timing_campaign;validate_timing_evidence |
| VER-REQ-101 | MANUAL | Verified | DigitalThread | verification_matrix;check_requirements |
| VER-REQ-102 | MANUAL | Verified | DigitalThread | check_requirements;test_traceability;update_traceability_baseline |
| VER-REQ-103 | PASS | Verified | AssuranceAssistant | assurance_assistant;assurance_assistant_eval |
| VER-REQ-104 | PASS | Verified | AssuranceAssistant | test_assurance_assistant;assurance_assistant_eval |
| VER-REQ-105 | PASS | Verified | AssuranceAssistant | generate_assurance_assistant_eval;assurance_assistant_eval |

## Scenario Evidence

| Scenario | Result | Requirements | Report |
|---|---|---|---|
| basic_command_fault | PASS | FSW-REQ-002, FSW-REQ-004, FSW-REQ-005, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_basic_command_fault_output.txt` |
| command_timestamp_guard | PASS | FSW-REQ-115, FSW-REQ-112, FSW-REQ-113, VER-REQ-002 | `reports/scenario_command_timestamp_guard_output.txt` |
| extended_modes | PASS | FSW-REQ-003, FSW-REQ-012, FSW-REQ-106, VER-REQ-002 | `reports/scenario_extended_modes_output.txt` |
| hil_smoke_test | PASS | FSW-REQ-002, FSW-REQ-004, FSW-REQ-005, FSW-REQ-106, VER-REQ-002, VER-REQ-004, VER-REQ-008 | `reports/scenario_hil_smoke_test_output.txt` |
| invalid_transition_rejected | PASS | FSW-REQ-003, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_invalid_transition_rejected_output.txt` |
| recovery_failure_failsafe | PASS | FSW-REQ-210, FSW-REQ-209, FSW-REQ-113, VER-REQ-002 | `reports/scenario_recovery_failure_failsafe_output.txt` |
| sensor_timeout_safe_mode | PASS | FSW-REQ-006, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_sensor_timeout_safe_mode_output.txt` |
| watchdog_timeout_safe_mode | PASS | FSW-REQ-007, FSW-REQ-106, VER-REQ-002, VER-REQ-004 | `reports/scenario_watchdog_timeout_safe_mode_output.txt` |

## Reverse CTest Allocation

Registered CTests: `20`

Every registered CTest name must appear in at least one verification-matrix case list.
