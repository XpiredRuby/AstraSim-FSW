# ASTRA-OS Software Requirements

## Scope and claim boundary

This document is the canonical requirements baseline for ASTRA-OS. The project is a safety-critical-inspired educational and portfolio platform. It is not certified, flight qualified, airworthy, or DO-178C compliant. A requirement is marked **Verified** only when repeatable evidence exists in the repository or current CI. **Implemented** means code and tests exist but the evidence package has not yet been dispositioned. **Historical** identifies preserved evidence from the predecessor project. **Planned** identifies unimplemented scope.

## Flight-software core

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| FSW-REQ-001 | Flight software shall initialize in BOOT mode. | Unit test | `mode_manager_tests`, `flight_software_app_tests` | Verified |
| FSW-REQ-002 | Flight software shall transition from BOOT to NOMINAL after an accepted SET_MODE command. | Unit test and scenario | `flight_software_app_tests`, `scenarios/basic_command_fault.yaml` | Verified |
| FSW-REQ-003 | Flight software shall reject prohibited mode transitions without changing the current mode. | Unit test and scenario | `mode_manager_tests`, `scenarios/invalid_transition_rejected.yaml` | Verified |
| FSW-REQ-004 | Flight software shall transition to DEGRADED_PAYLOAD after CPU_OVERLOAD is processed from NOMINAL. | Unit test and scenario | `command_processor_tests`, `scenarios/basic_command_fault.yaml` | Verified |
| FSW-REQ-005 | Flight software shall clear the active fault after an accepted CLEAR_FAULT command. | Unit test and scenario | `command_processor_tests`, `scenarios/basic_command_fault.yaml` | Verified |
| FSW-REQ-006 | Flight software shall enter SAFE after the health monitor detects a sensor timeout. | Unit test and scenario | `health_monitor_tests`, `scenarios/sensor_timeout_safe_mode.yaml` | Verified |
| FSW-REQ-007 | Flight software shall enter SAFE after the watchdog detects a deadline miss or execution timeout. | Unit test and scenario | `watchdog_tests`, `scenarios/watchdog_timeout_safe_mode.yaml` | Verified |
| FSW-REQ-008 | The core shall expose a deterministic tick-driven rate-group scheduler with explicit periods, phases, and deadlines. | Unit test | `rate_group_scheduler_tests` | Implemented |
| FSW-REQ-009 | The scheduler shall reject invalid rate-group configurations before execution. | Unit test | `rate_group_scheduler_tests` | Implemented |
| FSW-REQ-010 | The scheduler shall reject skipped, repeated, or regressed scheduler ticks without advancing state. | Unit test | `rate_group_scheduler_tests` | Implemented |
| FSW-REQ-011 | The scheduler shall record task releases, completions, overruns, and deadline misses deterministically. | Unit test | `rate_group_scheduler_tests` | Implemented |
| FSW-REQ-012 | The operational mode set shall include STANDBY and TEST without changing existing on-wire numeric values. | Design review and unit test | Not yet implemented | Planned |
| FSW-REQ-013 | The production app loop shall execute services through defined scheduler rate groups. | Integration test and timing report | Scheduler exists but is not yet integrated into `FlightSoftwareApp` | Planned |

## Command and telemetry

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| FSW-REQ-101 | Command packets shall use a fixed documented binary layout with magic, version, sequence, timestamp, command ID, argument, and CRC fields. | Unit test and inspection | `command_packet_tests`, `docs/command_packet.md` | Verified |
| FSW-REQ-102 | Command deserialization shall reject incorrect packet lengths. | Unit test | `command_packet_tests` exhaustive truncation and oversize cases | Verified |
| FSW-REQ-103 | Command deserialization shall reject invalid CRC values. | Unit test and controlled mutation | `command_packet_tests`, `tools/run_controlled_mutation.py` | Implemented |
| FSW-REQ-104 | Command deserialization shall reject invalid magic values even when the CRC is internally consistent. | Unit test | `command_packet_tests` | Verified |
| FSW-REQ-105 | Command deserialization shall reject unsupported protocol versions even when the CRC is internally consistent. | Unit test | `command_packet_tests` | Verified |
| FSW-REQ-106 | Command deserialization shall reject unknown command IDs even when the CRC is internally consistent. | Unit test | `command_packet_tests` | Verified |
| FSW-REQ-107 | Failed command deserialization shall not modify the caller-provided output object. | Unit test | `command_packet_tests` | Verified |
| FSW-REQ-108 | The command parser shall reject every single-bit corruption of the current 26-byte command packet. | Deterministic fault campaign | `command_packet_tests` 208-mutation campaign | Verified |
| FSW-REQ-109 | The flight-software boundary shall reject duplicate ground command sequence numbers without changing state. | Unit test | `flight_software_app_tests` | Implemented |
| FSW-REQ-110 | The flight-software boundary shall reject replayed or stale ground command sequence numbers without changing state. | Unit test | `flight_software_app_tests` | Implemented |
| FSW-REQ-111 | Ground sequence comparison shall accept unsigned 32-bit wrap from maximum to zero. | Unit test | `flight_software_app_tests` | Implemented |
| FSW-REQ-112 | A syntactically valid but semantically rejected command shall consume its sequence number. | Unit test | `flight_software_app_tests` | Implemented |
| FSW-REQ-113 | Telemetry shall acknowledge the sequence, command ID, and disposition of the last attempted ground command. | Unit test and scenario | `telemetry_packet_tests`, `flight_software_app_tests` | Verified |
| FSW-REQ-114 | Telemetry packets shall use CRC validation and reject invalid fields. | Unit test | `telemetry_packet_tests` | Verified |
| FSW-REQ-115 | Command freshness shall be bounded using a defined mission-time tolerance in addition to sequence checks. | Unit test and scenario | Timestamp is carried but freshness is not yet enforced | Planned |
| FSW-REQ-116 | Ground-command authorization shall be separated from CRC integrity checking. | Security design and adversarial test | UDP command link is currently unauthenticated | Planned |

## Health monitoring and FDIR

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| FSW-REQ-201 | Health monitoring shall classify nominal, warning, and critical CPU load states. | Unit test | `health_monitor_tests`, `flight_software_app_tests` | Verified |
| FSW-REQ-202 | Health monitoring shall classify nominal, warning, and critical memory load states. | Unit test | `health_monitor_tests` | Verified |
| FSW-REQ-203 | Health monitoring shall detect stale sensor data. | Unit test and scenario | `health_monitor_tests`, `scenarios/sensor_timeout_safe_mode.yaml` | Verified |
| FSW-REQ-204 | Health monitoring shall detect stale payload heartbeat data. | Unit test | `health_monitor_tests` | Verified |
| FSW-REQ-205 | Watchdog evaluation shall distinguish OK, WARNING, and EXPIRED states. | Unit test | `watchdog_tests` | Verified |
| FSW-REQ-206 | Internal health and watchdog faults shall bypass ground-command replay protection while retaining deterministic fault processing. | Integration test | `flight_software_app_tests` | Implemented |
| FSW-REQ-207 | Every supported fault shall have an explicit detection source, persistence rule, isolation result, response, recovery rule, telemetry indication, and linked verification case. | Requirements review and fault campaign | Current fault mapping is incomplete | Planned |
| FSW-REQ-208 | The system shall support at least ten independently verifiable FDIR cases. | Fault campaign | Fault codes exist but full detection and recovery evidence is incomplete | Planned |
| FSW-REQ-209 | Simultaneous faults shall be resolved using a documented deterministic priority policy. | Unit test and scenario | No explicit priority manager yet | Planned |
| FSW-REQ-210 | Repeated recovery failures shall force a bounded fail-safe disposition. | Scenario test | Not yet implemented | Planned |

## Verification and assurance

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| VER-REQ-001 | The repository shall provide a repeatable clean local build and unit-test command. | Script execution | `ci/run_local_tests.sh` | Verified |
| VER-REQ-002 | The repository shall provide deterministic YAML scenario execution. | Tool execution | `tools/run_scenario.py` | Verified |
| VER-REQ-003 | The repository shall provide one command for the complete existing scenario and regression suite. | Tool execution | `tools/run_all_scenarios.py` | Verified |
| VER-REQ-004 | Scenario runs shall produce human-readable evidence reports. | Report inspection | `reports/scenario_*_output.txt` | Verified |
| VER-REQ-005 | Automated traceability checks shall identify unknown requirement references and missing scenario evidence. | Tool test and execution | `tools/check_requirements.py` | Verified |
| VER-REQ-006 | Monte Carlo regression shall use an explicit reproducible seed. | Tool execution | `tools/run_monte_carlo.py` default seed `20260626` | Verified |
| VER-REQ-007 | The repository shall package the software and instructions needed for Raspberry Pi deployment. | Package generation | `tools/package_pi_deployment.sh` | Verified |
| VER-REQ-008 | Preserved Raspberry Pi HIL claims shall remain traceable to recorded evidence and shall not be represented as newly rerun evidence. | Evidence review | `evidence/pi_ctest_results.txt`, `evidence/pi_command_telemetry_demo.txt` | Historical |
| VER-REQ-009 | Supported compiler builds shall treat baseline warnings as errors by default. | CI build | `ASTRA_WARNINGS_AS_ERRORS`, C++ Build and Unit Tests workflow | Implemented |
| VER-REQ-010 | CI shall execute the C++ test suite under AddressSanitizer and UndefinedBehaviorSanitizer. | CI job | `.github/workflows/assurance.yml` | Implemented |
| VER-REQ-011 | CI shall generate structural coverage evidence with test code excluded from the reported core metric. | CI job and artifact | `.github/workflows/assurance.yml` | Implemented |
| VER-REQ-012 | CI shall run clang-tidy with findings treated as errors. | CI job | `.github/workflows/assurance.yml` | Implemented |
| VER-REQ-013 | CI shall compile all Python tools and statically inspect shell scripts. | CI job | `.github/workflows/assurance.yml` | Implemented |
| VER-REQ-014 | The command-packet test suite shall kill a controlled defect that disables CRC rejection. | Controlled mutation | `tools/run_controlled_mutation.py` | Implemented |
| VER-REQ-015 | Each full verification run shall produce a provenance manifest containing commit, repository state, host, toolchain, build options, commands, and input hashes. | CI artifact inspection | `tools/generate_baseline_manifest.py`, Full Verification workflow | Implemented |
| VER-REQ-016 | Coverage shall be reported by production module with documented exclusions and no unsupported numerical claims. | Coverage report review | CI currently captures aggregate LCOV data | Planned |
| VER-REQ-017 | The project shall provide a bounded fuzzing target for command decoding and preserve a reproducible seed corpus. | Fuzz execution | Deterministic corruption tests exist; coverage-guided fuzzing is not yet integrated | Planned |
| VER-REQ-018 | Timing, jitter, CPU, memory, and soak evidence shall include exact workload, host, duration, and toolchain provenance. | Performance campaign | Not yet generated for ASTRA-OS | Planned |

## Digital thread and governed assistant

| ID | Requirement | Verification Method | Current Evidence | Status |
|---|---|---|---|---|
| VER-REQ-101 | Every implemented software requirement shall link to an architecture component and at least one verification case. | Traceability checker | Initial verification matrix | Implemented |
| VER-REQ-102 | The checker shall identify orphan requirements, tests without requirements, stale interface revisions, and changed requirements awaiting review. | Tool test | Existing checker only covers part of this scope | Planned |
| VER-REQ-103 | The governed assistant shall only retrieve approved project sources and invoke explicitly allow-listed verification tools. | Frozen adversarial evaluation | Deferred until flight-software assurance baseline is stable | Planned |
| VER-REQ-104 | The governed assistant shall not merge code, issue unrestricted shell commands, command hardware, or automatically mark requirements verified. | Permission test | Deferred | Planned |
| VER-REQ-105 | The governed assistant evaluation set shall contain at least 100 frozen cases with quantitative results. | Evaluation harness | Deferred | Planned |
