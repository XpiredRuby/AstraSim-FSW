# ASTRA-OS / AstraSim-FSW

[![C++ Build and Unit Tests](https://github.com/XpiredRuby/AstraSim-FSW/actions/workflows/unit_tests.yml/badge.svg)](https://github.com/XpiredRuby/AstraSim-FSW/actions/workflows/unit_tests.yml)

ASTRA-OS is an educational C++/Python spacecraft-style flight-software and verification platform. It demonstrates operating modes, binary UDP command and telemetry protocols, command validation, health monitoring, fault detection/isolation/recovery, watchdog behavior, deterministic rate-group scheduling, configuration locking, Raspberry Pi execution, requirements traceability, and automated assurance evidence.

The embedded core is C++17. Python provides ground-side command injection, telemetry decoding, deterministic scenario execution, protocol checks, randomized regression, evidence generation, and packaging.

## Verified Raspberry Pi Status

The repair and assurance campaign was executed natively on a Raspberry Pi running Ubuntu 24.04 on `aarch64`.

| Verification area | Result |
|---|---|
| Native Raspberry Pi build | PASS |
| CTest suites | 18/18 passed |
| Deterministic YAML scenarios | 5/5 passed |
| Protocol conformance | 24/24 passed |
| Requirement traceability | 0 failures; 0 broken references |
| Seeded Monte Carlo regression | 25/25 passed; seed `20260626` |
| ASan/UBSan build and tests | PASS |
| Controlled mutation check | PASS |
| Pi deployment package | PASS |
| Nominal timing campaign | 250,000 ticks; 0 deadline misses |
| Controlled-overrun campaign | 100,000 ticks; 99 injected misses detected |
| Soak campaign | 1,000,000 ticks; 0 deadline misses |
| Final assurance workflow | PASS |

The software-under-test for the final Pi campaign was commit:

```text
395f1e028a3ab6bf46438b4dbd5bad256eced0cd
```

Full report:

- [`reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md`](reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md)

## Scope and Claim Boundary

ASTRA-OS is a portfolio and educational project. The preserved results demonstrate native Raspberry Pi execution and repeatable software verification under the documented conditions.

They do **not** establish:

- certification;
- flight qualification;
- hard-real-time guarantees;
- airworthiness;
- production readiness;
- radiation tolerance;
- compatibility with spacecraft hardware.

The timing campaigns are host faster-than-real-time execution measurements. They are not real-time qualification or worst-case execution-time proof.

## Implemented Capabilities

- BOOT, NOMINAL, DEGRADED, SAFE, and RECOVERY operating modes
- controlled and rejected mode transitions
- binary command and telemetry packet formats
- CRC-16-CCITT validation
- UDP command receiver and telemetry sender
- Python command sender and telemetry decoder
- accepted, invalid, duplicate, replayed, stale, future, and guard-configuration command dispositions
- command freshness and sequence guarding
- health monitoring for CPU, memory, sensor, payload, and loop state
- watchdog timeout detection
- fault disposition and simultaneous-fault behavior
- deterministic rate-group scheduling
- application/executive configuration validation
- configuration locking
- event logging and event integration
- YAML scenario execution
- seeded Monte Carlo regression
- C++/Python protocol-manifest conformance checking
- requirement-to-evidence traceability checking
- ASan/UBSan assurance builds
- controlled command-packet mutation
- Raspberry Pi deployment packaging
- machine-readable provenance and assurance summaries

## System Flow

```text
Python command sender
        |
        v
UDP command packet
        |
        v
C++ UDP command receiver
        |
        v
Command packet decoder + CRC + command guard
        |
        v
FlightSoftwareExecutive / FlightSoftwareApp
        |
        +--> RateGroupScheduler
        +--> Watchdog
        +--> HealthMonitor
        +--> CommandProcessor
        +--> FDIRManager
        +--> ModeManager
        +--> EventLogger
        |
        v
Telemetry packet encoder + CRC
        |
        v
C++ UDP telemetry sender
        |
        v
Python telemetry receiver / scenario verifier
```

## Main Native Targets

| Target | Purpose |
|---|---|
| `astra_fsw` | Basic mode/fault flight-software demonstration |
| `astra_fsw_telemetry_demo` | UDP telemetry demonstration |
| `astra_fsw_command_telemetry_demo` | Integrated UDP command, mode/fault, acknowledgement, and telemetry demonstration |
| `astra_timing_campaign` | Faster-than-real-time host execution and scheduler-miss evidence generator |

## Native Raspberry Pi Build

```bash
cmake -S . -B build-pi -DCMAKE_BUILD_TYPE=Release
cmake --build build-pi --parallel
ctest --test-dir build-pi --output-on-failure
```

Expected test count for this branch:

```text
18/18 suites passing
```

## Complete Assurance Workflow

Run the full deterministic, randomized, sanitizer, mutation, packaging, traceability, protocol, and provenance workflow against the Pi build:

```bash
python3 tools/run_astra_os_assurance.py --build-dir build-pi
```

The machine-readable summary is written to:

```text
reports/latest/assurance_summary.json
```

## Deterministic Scenarios

Run all declared scenarios and verification checks without rebuilding or repeating Monte Carlo/package generation:

```bash
python3 tools/run_all_scenarios.py \
  --build-dir build-pi \
  --skip-build \
  --skip-monte-carlo \
  --skip-pi-package
```

Declared scenarios:

- `basic_command_fault.yaml`
- `hil_smoke_test.yaml`
- `invalid_transition_rejected.yaml`
- `sensor_timeout_safe_mode.yaml`
- `watchdog_timeout_safe_mode.yaml`

## Seeded Monte Carlo Regression

```bash
python3 tools/run_monte_carlo.py \
  --build-dir build-pi \
  --trials 25 \
  --seed 20260626
```

## Raspberry Pi Deployment Package

```bash
bash tools/package_pi_deployment.sh --build-dir build-pi
```

Generated archive:

```text
dist/astrasim-fsw-pi.tar.gz
```

Package generation and native-hardware execution are separate claims. See the deployment report and the final Pi verification report for their respective evidence boundaries.

## Verification Evidence

| Evidence | Purpose |
|---|---|
| [`reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md`](reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md) | Final Pi verification, defects, repairs, metrics, limitations, and reproduction commands |
| [`reports/latest/assurance_summary.json`](reports/latest/assurance_summary.json) | Machine-readable final assurance-stage results |
| [`reports/latest/baseline_manifest.json`](reports/latest/baseline_manifest.json) | Commit, platform, toolchain, commands, and hashed input provenance |
| [`reports/requirement_check_report.md`](reports/requirement_check_report.md) | Requirement disposition and traceability integrity |
| [`reports/monte_carlo_report.md`](reports/monte_carlo_report.md) | Seeded 25-trial randomized regression |
| [`reports/pi_deployment_package_report.md`](reports/pi_deployment_package_report.md) | Deployment-package contents and archive checksum |
| [`reports/latest/protocol_conformance.json`](reports/latest/protocol_conformance.json) | C++/Python protocol consistency checks |
| [`reports/pi-hil/`](reports/pi-hil/) | Nominal, controlled-overrun, soak, resource, and final CTest evidence |
| `reports/scenario_*_output.txt` | Deterministic command/fault scenario transcripts |

## Key Python Tools

| Tool | Purpose |
|---|---|
| `tools/send_command.py` | Encode and send UDP commands |
| `tools/telemetry_receiver.py` | Receive and decode telemetry |
| `tools/run_scenario.py` | Execute one deterministic YAML scenario |
| `tools/run_all_scenarios.py` | Run deterministic scenarios and repository checks |
| `tools/run_monte_carlo.py` | Run reproducible randomized scenarios |
| `tools/check_protocol_conformance.py` | Compare protocol definitions across C++/Python/manifest sources |
| `tools/check_requirements.py` | Validate requirement and evidence traceability |
| `tools/run_controlled_mutation.py` | Confirm detection of a controlled packet mutation |
| `tools/run_astra_os_assurance.py` | Orchestrate the complete assurance workflow |
| `tools/validate_timing_evidence.py` | Validate timing-evidence structure and expected miss behavior |
| `tools/package_pi_deployment.sh` | Build and package the target bundle from an explicit build directory |

## Documentation

| Document | Purpose |
|---|---|
| `docs/ARCHITECTURE.md` | System architecture and component boundaries |
| `docs/REQUIREMENTS.md` | Governed requirements set |
| `docs/VERIFICATION_MATRIX.csv` | Requirement-to-component-to-evidence mapping |
| `docs/ASSURANCE.md` | Assurance workflow and evidence policy |
| `docs/FDIR_MATRIX.md` | Fault disposition model |
| `docs/RISKS_AND_BLOCKERS.md` | Known limitations and unresolved work |
| `docs/pi_deployment.md` | Raspberry Pi packaging and deployment guidance |
| `docs/monte_carlo.md` | Randomized regression guidance |

## Repository Safety Note

The Pi verification work was performed only in:

```text
/home/xpired/ghost_ws/tools/astra-os-hil
```

The older preservation worktree containing historical uncommitted files was intentionally left untouched.
