# ASTRA-OS Software Requirements

## Scope and claim boundary

This document is the canonical requirements baseline for ASTRA-OS. The project is a safety-critical-inspired educational and portfolio platform. It is not certified, flight qualified, airworthy, production ready, or DO-178C compliant. A requirement is marked **Verified** only when repeatable evidence exists in the repository or current CI. **Implemented** means code and tests exist but the evidence package has not yet been dispositioned. **Historical** identifies preserved predecessor evidence. **Planned** identifies unimplemented scope.

## Flight-software core

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| FSW-REQ-001 | Flight software shall initialize in BOOT mode. | Unit test | `mode_manager_tests`, `flight_software_app_tests` | Verified |
| FSW-REQ-002 | Flight software shall transition from BOOT to NOMINAL after an accepted SET_MODE command. | Unit test and scenario | `flight_software_app_tests`, `scenarios/basic_command_fault.yaml` | Verified |
| FSW-REQ-003 | Flight software shall reject prohibited mode transitions without changing the current mode. | Unit test and scenario | `mode_manager_tests`, `scenarios/invalid_transition_rejected.yaml`, `scenarios/extended_modes.yaml` | Verified |
| FSW-REQ-004 | Flight software shall transition to DEGRADED_PAYLOAD after CPU_OVERLOAD is processed from NOMINAL. | Unit test and scenario | `command_processor_tests`, `scenarios/basic_command_fault.yaml` | Verified |
| FSW-REQ-005 | Flight software shall clear the active fault after an accepted CLEAR_FAULT command. | Unit test and scenario | `command_processor_tests`, `scenarios/basic_command_fault.yaml` | Verified |
| FSW-REQ-006 | Flight software shall enter SAFE after the health monitor detects a sensor timeout. | Unit test and scenario | `health_monitor_tests`, `scenarios/sensor_timeout_safe_mode.yaml` | Verified |
| FSW-REQ-007 | Flight software shall enter SAFE after the watchdog detects a deadline miss or execution timeout. | Unit test and scenario | `watchdog_tests`, `scenarios/watchdog_timeout_safe_mode.yaml` | Verified |
| FSW-REQ-008 | The core shall expose a deterministic tick-driven rate-group scheduler with explicit periods, phases, and deadlines. | Unit test | `rate_group_scheduler_tests`, `flight_software_executive_tests` | Verified |
| FSW-REQ-009 | The scheduler shall reject invalid rate-group configurations before execution. | Unit test | `rate_group_scheduler_tests`, `flight_software_executive_tests` | Verified |
| FSW-REQ-010 | The scheduler shall reject skipped, repeated, or regressed scheduler ticks without advancing state. | Unit test | `rate_group_scheduler_tests` | Verified |
| FSW-REQ-011 | The scheduler shall record task releases, completions, overruns, and deadline misses deterministically. | Unit test and timing campaign | `rate_group_scheduler_tests`, `reports/pi-hil/` | Verified |
| FSW-REQ-012 | The operational mode set shall include STANDBY and TEST without changing existing on-wire numeric values. | Unit test, protocol check, and scenario | `extended_mode_tests`, `scenarios/extended_modes.yaml`, `config/protocol_manifest.json` | Verified |
| FSW-REQ-013 | The production executive shall execute flight-software services through defined scheduler rate groups. | Integration test and timing report | `flight_software_executive_tests`, `reports/pi-hil/` | Verified |

## Command and telemetry

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| FSW-REQ-101 | Command packets shall use a fixed documented binary layout with magic, version, sequence, timestamp, command ID, argument, and CRC fields. | Unit test and inspection | `command_packet_tests`, `docs/command_packet.md` | Verified |
| FSW-REQ-102 | Command deserialization shall reject incorrect packet lengths. | Unit test | `command_packet_tests` exhaustive truncation and oversize cases | Verified |
| FSW-REQ-103 | Command deserialization shall reject invalid CRC values. | Unit test and controlled mutation | `command_packet_tests`, `tools/run_controlled_mutation.py` | Verified |
| FSW-REQ-104 | Command deserialization shall reject invalid magic values even when the CRC is internally consistent. | Unit test | `command_packet_tests` | Verified |
| FSW-REQ-105 | Command deserialization shall reject unsupported protocol versions even when the CRC is internally consistent. | Unit test | `command_packet_tests` | Verified |
| FSW-REQ-106 | Command deserialization shall reject unknown command IDs even when the CRC is internally consistent. | Unit test | `command_packet_tests` | Verified |
| FSW-REQ-107 | Failed command deserialization shall not modify the caller-provided output object. | Unit test | `command_packet_tests` | Verified |
| FSW-REQ-108 | The command parser shall reject every single-bit corruption of the current 26-byte command packet. | Deterministic fault campaign | `command_packet_tests` 208-mutation campaign | Verified |
| FSW-REQ-109 | The flight-software boundary shall reject duplicate ground command sequence numbers without changing state. | Unit test | `flight_software_app_tests`, `ground_command_guard_tests` | Verified |
| FSW-REQ-110 | The flight-software boundary shall reject replayed or stale ground command sequence numbers without changing state. | Unit test | `flight_software_app_tests`, `ground_command_guard_tests` | Verified |
| FSW-REQ-111 | Ground sequence comparison shall accept unsigned 32-bit wrap from maximum to zero. | Unit test | `flight_software_app_tests`, `ground_command_guard_tests` | Verified |
| FSW-REQ-112 | A syntactically valid but semantically or policy rejected command shall consume its accepted sequence number. | Unit test and scenario | `flight_software_app_tests`, `command_authorizer_tests`, `scenarios/command_timestamp_guard.yaml` | Verified |
| FSW-REQ-113 | Telemetry shall acknowledge the sequence, command ID, and disposition of the last attempted ground command. | Unit test and scenario | `telemetry_packet_tests`, `flight_software_app_tests`, deterministic scenarios | Verified |
| FSW-REQ-114 | Telemetry packets shall use CRC validation and reject invalid fields. | Unit test | `telemetry_packet_tests` | Verified |
| FSW-REQ-115 | Command freshness shall be bounded using configured maximum age and future-skew tolerances in addition to sequence checks. | Unit test and scenario | `ground_command_guard_tests`, `scenarios/command_timestamp_guard.yaml` | Verified |
| FSW-REQ-116 | Ground-command authorization policy shall be separated from packet CRC integrity checking and shall reject disabled commands before state mutation. | Unit test and design review | `command_authorizer_tests`, `docs/command_authorization.md` | Verified |

The authorization policy controls which already-decoded commands may execute. It does not authenticate a network sender and is not a cryptographic security mechanism.

## Health monitoring and FDIR

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| FSW-REQ-201 | Health monitoring shall classify nominal, warning, and critical CPU load states. | Unit test | `health_monitor_tests`, `flight_software_app_tests` | Verified |
| FSW-REQ-202 | Health monitoring shall classify nominal, warning, and critical memory load states. | Unit test | `health_monitor_tests` | Verified |
| FSW-REQ-203 | Health monitoring shall detect stale sensor data. | Unit test and scenario | `health_monitor_tests`, `scenarios/sensor_timeout_safe_mode.yaml` | Verified |
| FSW-REQ-204 | Health monitoring shall detect stale payload heartbeat data. | Unit test | `health_monitor_tests` | Verified |
| FSW-REQ-205 | Watchdog evaluation shall distinguish OK, WARNING, and EXPIRED states. | Unit test | `watchdog_tests` | Verified |
| FSW-REQ-206 | Internal health and watchdog faults shall bypass ground-command replay and authorization policy while retaining deterministic fault processing. | Integration test | `flight_software_app_tests`, `simultaneous_fault_tests` | Verified |
| FSW-REQ-207 | Every supported fault shall have an explicit detection source or policy source, persistence intent, isolation result, response, recovery rule, telemetry indication, and linked verification case. | Requirements review and fault campaign | `docs/FDIR_MATRIX.md`, `fdir_manager_tests`, `reports/fdir_campaign_report.md` | Verified |
| FSW-REQ-208 | The system shall support at least ten independently verifiable FDIR policy cases. | Fault campaign | `tools/run_fdir_campaign.py`, `reports/fdir_campaign_report.md` | Verified |
| FSW-REQ-209 | Simultaneous faults shall be resolved using a documented deterministic priority and tie-break policy. | Unit test and integration test | `fdir_manager_tests`, `simultaneous_fault_tests`, `docs/FDIR_MATRIX.md` | Verified |
| FSW-REQ-210 | Repeated failed exits from RECOVERY shall force a bounded SAFE disposition. | Unit test and scenario | `recovery_supervisor_tests`, `scenarios/recovery_failure_failsafe.yaml` | Verified |
| FSW-REQ-211 | Significant command dispositions, mode transitions, and fault transitions shall be recorded in a typed event log. | Unit and integration test | `event_logger_tests`, `event_integration_tests` | Verified |
| FSW-REQ-212 | The event log shall have bounded capacity and shall report dropped-record count. | Unit test | `event_logger_tests` | Verified |

Fault injection verifies policy response paths. It does not prove that every corresponding physical detector, persistence dwell, or hardware recovery mechanism exists.

## Configuration control

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| FSW-REQ-301 | System configuration shall use a versioned schema and reject invalid schema versions or values before activation. | Unit test | `configuration_service_tests` | Verified |
| FSW-REQ-302 | Configuration updates shall require the expected active revision and a strictly increasing candidate revision. | Unit test | `configuration_service_tests` | Verified |
| FSW-REQ-303 | A valid configuration shall support an irreversible runtime lock that rejects later updates. | Unit test | `configuration_service_tests` | Verified |

## Verification and assurance

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| VER-REQ-001 | The repository shall provide a repeatable clean local build and unit-test command. | Script execution | `ci/run_local_tests.sh` | Verified |
| VER-REQ-002 | The repository shall provide deterministic YAML scenario execution. | Tool execution | `tools/run_scenario.py` | Verified |
| VER-REQ-003 | The repository shall provide one command for the complete scenario and regression suite. | Tool execution | `tools/run_all_scenarios.py` | Verified |
| VER-REQ-004 | Scenario and campaign runs shall produce human-readable evidence reports. | Report inspection | `reports/scenario_*_output.txt`, `reports/fdir_campaign_report.md` | Verified |
| VER-REQ-005 | Automated traceability checks shall identify unknown requirement references and missing scenario evidence. | Tool test and execution | `tools/check_requirements.py` | Verified |
| VER-REQ-006 | Monte Carlo regression shall use an explicit reproducible seed. | Tool execution | `tools/run_monte_carlo.py` default seed `20260626` | Verified |
| VER-REQ-007 | The repository shall package the software and instructions needed for Raspberry Pi deployment. | Package generation | `tools/package_pi_deployment.sh` | Verified |
| VER-REQ-008 | Raspberry Pi claims shall remain traceable to commit and platform evidence and historical evidence shall not be presented as current execution. | Evidence review | `reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md`, `reports/pi-hil/` | Verified |
| VER-REQ-009 | Supported compiler builds shall treat baseline warnings as errors by default. | CI and native build | `ASTRA_WARNINGS_AS_ERRORS`, GitHub Actions | Verified |
| VER-REQ-010 | CI and the assurance workflow shall execute the C++ test suite under AddressSanitizer and UndefinedBehaviorSanitizer. | CI job and assurance report | `.github/workflows/assurance.yml`, `reports/latest/assurance_summary.json` | Verified |
| VER-REQ-011 | CI shall generate structural coverage evidence with test code excluded from the production metric. | CI job and artifact | `.github/workflows/assurance.yml` | Verified |
| VER-REQ-012 | CI shall run clang-tidy with findings treated as errors. | CI job | `.github/workflows/assurance.yml` | Verified |
| VER-REQ-013 | CI shall compile all Python tools, execute Python tool tests, and statically inspect shell scripts. | CI job | `.github/workflows/assurance.yml` | Verified |
| VER-REQ-014 | The command-packet test suite shall kill a controlled defect that disables CRC rejection. | Controlled mutation | `tools/run_controlled_mutation.py`, `reports/latest/controlled_mutation.log` | Verified |
| VER-REQ-015 | Each full verification run shall produce a provenance manifest containing commit, repository state, host, toolchain, build options, commands, and input hashes. | Artifact inspection | `tools/generate_baseline_manifest.py`, `reports/latest/baseline_manifest.json` | Verified |
| VER-REQ-016 | Coverage shall be reported by production module with documented exclusions and no unsupported adequacy claims. | Coverage report review | `tools/summarize_lcov.py`, `.github/workflows/assurance.yml` | Verified |
| VER-REQ-017 | The project shall provide a bounded coverage-guided command-decoding fuzzer and preserve a reproducible seed corpus. | Fuzz execution | `fsw/fuzz/fuzz_command_packet.cpp`, `tools/generate_command_fuzz_corpus.py`, `.github/workflows/assurance.yml` | Verified |
| VER-REQ-018 | Timing, jitter, CPU, memory, and soak evidence shall include exact workload, host, duration, and toolchain provenance. | Performance campaign | `reports/pi-hil/`, `.github/workflows/performance.yml` | Verified |

## Digital thread and governed assistant

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| VER-REQ-101 | Every implemented software requirement shall link to an architecture component and at least one verification case. | Traceability checker | `docs/VERIFICATION_MATRIX.csv`, `tools/check_requirements.py` | Verified |
| VER-REQ-102 | The checker shall identify orphan requirements, registered tests without requirements, changed requirements awaiting review, and stale controlled-interface revisions. | Tool test and execution | `tools/check_requirements.py`, `tools/tests/test_traceability.py`, `config/traceability_baseline.json` | Verified |
| VER-REQ-103 | The governed assurance assistant shall retrieve only approved project sources and invoke only explicitly allow-listed verification tools. | Frozen permission evaluation | `tools/assurance_assistant.py`, `config/assurance_assistant_policy.json`, `reports/assurance_assistant_eval.md` | Verified |
| VER-REQ-104 | The governed assurance assistant shall deny merge, push, unrestricted shell, hardware command, write, deletion, and automatic requirement-verification actions. | Permission test and frozen evaluation | `tools/tests/test_assurance_assistant.py`, `reports/assurance_assistant_eval.md` | Verified |
| VER-REQ-105 | The governed assurance-assistant evaluation set shall contain at least 100 frozen quantitative cases. | Evaluation harness | `config/assurance_assistant_eval.json`, `tools/run_assurance_assistant_eval.py`, `reports/assurance_assistant_eval.md` | Verified |
