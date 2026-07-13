# ASTRA-OS Raspberry Pi Verification, Assurance, and Release-Closure Report

Date: 2026-07-13

## 1. Executive Summary

ASTRA-OS was built and verified natively on a Raspberry Pi running Ubuntu 24.04 on `aarch64`. The software-under-test was branch `astra-os/pi-hil-build-fix` at commit `395f1e028a3ab6bf46438b4dbd5bad256eced0cd`.

The final managed assurance workflow completed with `overall_status: passed`. It included the full deterministic and randomized verification campaign, a warnings-as-errors ASan/UBSan build and test pass, a controlled mutation test, deployment-package generation, requirement traceability, protocol conformance, and a provenance manifest.

Primary results:

- Native Raspberry Pi build: PASS
- CTest: 18/18 suites passed
- Deterministic scenarios: 5/5 passed
- Protocol conformance: 24/24 checks passed
- Requirement traceability: 0 failed requirements and 0 broken traceability references
- Seeded Monte Carlo regression: 25/25 trials passed with seed `20260626`
- ASan/UBSan build and tests: PASS
- Controlled mutation check: PASS
- Raspberry Pi deployment package: PASS
- Timing-evidence validators: PASS for nominal, controlled-overrun, and soak campaigns
- Final assurance summary: PASS

This is educational and portfolio-focused evidence. It does not establish certification, flight qualification, hard-real-time guarantees, airworthiness, production readiness, or compatibility with spacecraft hardware.

## 2. Software Baseline

| Field | Value |
|---|---|
| Repository | `XpiredRuby/AstraSim-FSW` |
| Branch | `astra-os/pi-hil-build-fix` |
| Software-under-test commit | `395f1e028a3ab6bf46438b4dbd5bad256eced0cd` |
| Original frozen baseline | `805849306dd0472b87431bac4a427eda145f9f9e` |
| Active Pi worktree | `/home/xpired/ghost_ws/tools/astra-os-hil` |
| Native build directory | `build-pi` |

The older preservation worktree at `/home/xpired/ghost_ws/tools/astra-os-work` was not modified, cleaned, reset, committed, or deleted.

## 3. Raspberry Pi Platform Provenance

| Field | Observed value |
|---|---|
| Operating system | Ubuntu 24.04 |
| Kernel | `6.8.0-1060-raspi` |
| Architecture | `aarch64` |
| Processor interface | `aarch64` |
| Logical CPUs | 4 |
| Total RAM | 3,966,840,832 bytes, approximately 3.69 GiB |
| Swap | 0 bytes |
| CMake | 3.28.3 |
| C++ compiler | GCC/G++ 13.3.0 |
| Python | 3.12.3 |
| Git | 2.43.0 |

The restricted connector did not expose the exact Raspberry Pi board/SoC model string. This report therefore records the verified architecture and logical CPU count and intentionally does not guess the hardware model.

Machine-readable provenance is preserved in:

- `reports/latest/baseline_manifest.json`

## 4. Native Build and Unit/Integration Tests

A clean Release configuration and complete native rebuild were performed in `build-pi`.

CTest result:

```text
100% tests passed, 0 tests failed out of 18
```

The 18 suites covered:

1. mode manager
2. telemetry packet
3. UDP telemetry sender
4. command packet
5. command processor
6. UDP command receiver
7. flight-software app
8. health monitor
9. watchdog
10. rate-group scheduler
11. extended modes
12. FDIR manager
13. simultaneous faults
14. flight-software executive
15. event logger
16. event integration
17. ground-command guard
18. configuration service

Evidence:

- `reports/pi-hil/final-ctest.txt`
- `reports/latest/full_verification.log`
- `reports/latest/test_sanitizers.log`

## 5. Deterministic Command/Fault Scenarios

All five declared YAML scenarios passed against the native `build-pi` executable set:

- `scenarios/basic_command_fault.yaml`
- `scenarios/hil_smoke_test.yaml`
- `scenarios/invalid_transition_rejected.yaml`
- `scenarios/sensor_timeout_safe_mode.yaml`
- `scenarios/watchdog_timeout_safe_mode.yaml`

Verified behavior included:

- `BOOT -> NOMINAL`
- external UDP command acceptance
- CPU-overload injection and `NOMINAL -> DEGRADED_PAYLOAD`
- fault clearing
- invalid transition rejection
- sensor-timeout transition to `SAFE`
- watchdog-deadline-miss transition to `SAFE`
- command acknowledgement sequence, command, and status fields

Evidence:

- `reports/scenario_basic_command_fault_output.txt`
- `reports/scenario_hil_smoke_test_output.txt`
- `reports/scenario_invalid_transition_rejected_output.txt`
- `reports/scenario_sensor_timeout_safe_mode_output.txt`
- `reports/scenario_watchdog_timeout_safe_mode_output.txt`

## 6. Protocol, Traceability, and Randomized Regression

### Protocol conformance

Result: 24/24 checks passed.

The conformance tool compared C++ and Python definitions for mode, fault, command, status, packet magic, version, type, and packet size.

Evidence:

- `reports/latest/protocol_conformance.json`

### Requirement traceability

Result:

```text
PASS=9
MANUAL=37
PLANNED=15
HISTORICAL=1
FAIL=0
TRACEABILITY_PROBLEMS=0
```

`MANUAL` means a requirement has a matrix allocation but was not automatically dispositioned by the scenario-report logic. It is not treated as an automatic pass.

Evidence:

- `reports/requirement_check_report.md`
- `docs/REQUIREMENTS.md`
- `docs/VERIFICATION_MATRIX.csv`

### Monte Carlo regression

Result: 25/25 trials passed using explicit seed `20260626`.

The randomized campaign exercised nominal transitions, invalid transition rejection, CPU fault/clear behavior, sensor timeout, watchdog timeout, randomized ports, and randomized timeout trigger locations.

Evidence:

- `reports/monte_carlo_report.md`

## 7. Sanitizers, Mutation, and Assurance Orchestration

The complete managed assurance workflow passed:

- full verification using `build-pi`
- warnings-as-errors sanitizer configuration
- ASan/UBSan build
- ASan/UBSan CTest execution
- controlled command-packet mutation
- provenance-manifest generation

Evidence:

- `reports/latest/assurance_summary.json`
- `reports/latest/final_assurance_process.log`
- `reports/latest/configure_sanitizers.log`
- `reports/latest/build_sanitizers.log`
- `reports/latest/test_sanitizers.log`
- `reports/latest/controlled_mutation.log`
- `reports/latest/baseline_manifest.json`

The final summary records `overall_status: passed` and return code 0 for every stage.

## 8. Raspberry Pi Timing and Resource Campaigns

These campaigns provide host faster-than-real-time execution evidence on the Raspberry Pi. They are not hard-real-time, HIL, certification, or flight-qualification evidence.

### Nominal campaign

- ticks: 250,000
- simulated mission step: 10 ms
- scheduler deadline misses: 0
- mean measured step: 708.514 ns
- p99: 889 ns
- maximum: 115,220 ns
- peak RSS: 7,040 KiB
- final mode: `NOMINAL`
- final fault: `NONE`
- collection-session temperature observation: approximately 33.1°C to 34.5°C

Evidence:

- `reports/pi-hil/nominal-timing.json`
- `reports/pi-hil/nominal-resource-usage.txt`

### Controlled-overrun campaign

- ticks: 100,000
- simulated mission step: 10 ms
- synthetic overload interval: every 1,000 ticks
- scheduler deadline misses detected: 99
- mean measured step: 739.286 ns
- p99: 1,222 ns
- maximum: 104,924 ns
- peak RSS: 4,608 KiB
- final mode: `NOMINAL`
- final fault: `NONE`

Evidence:

- `reports/pi-hil/controlled-overrun-timing.json`
- `reports/pi-hil/controlled-overrun-resource-usage.txt`

### One-million-cycle soak

- ticks: 1,000,000
- simulated mission step: 10 ms
- scheduler deadline misses: 0
- mean measured step: 722.098 ns
- p99: 963 ns
- maximum: 111,220 ns
- peak RSS: 18,688 KiB
- final mode: `NOMINAL`
- final fault: `NONE`
- collection-session temperature observation: approximately 33.6°C to 35.0°C

Evidence:

- `reports/pi-hil/soak-timing.json`
- `reports/pi-hil/soak-resource-usage.txt`

All three JSON files passed `tools/validate_timing_evidence.py` with the appropriate expected-miss disposition.

## 9. Deployment Package

The Raspberry Pi deployment archive was generated explicitly from `build-pi`:

- archive: `dist/astrasim-fsw-pi.tar.gz`
- reported size: 56K
- SHA256: `d00ddff08083217b03982dfccfa3667f47ae6d58c8d794927282e96d39813adb`

The package includes target binaries, command/telemetry tools, scenario support, the protocol manifest, requirements, and the verification matrix.

Evidence:

- `reports/pi_deployment_package_report.md`

Package generation proves that the bundle can be assembled. Hardware-execution claims are supported separately by the native Pi build, tests, scenarios, timing campaigns, and provenance evidence above.

## 10. Defects Discovered During Native Pi Validation

Native Pi execution exposed integration defects that earlier non-target work had not revealed:

1. Missing core sources in the CMake library target:
   - `ground_command_guard.cpp`
   - `configuration_service.cpp`
2. Missing CMake targets and CTest registration for ground-command guard and configuration service tests.
3. Missing private state in `ConfigurationService`.
4. Missing `<utility>` include required by the configuration implementation.
5. `FlightSoftwareExecutiveConfig` did not carry the application configuration into `FlightSoftwareApp`.
6. Executive validity and validation errors considered the scheduler but not the application.
7. Cross-machine command timestamps incorrectly compared unrelated `steady_clock`/`monotonic` epochs between laptop and Pi.
8. Scenario and HIL runners assumed a fixed `build/` directory.
9. Monte Carlo and deployment packaging did not propagate the selected build directory.
10. Python telemetry status mapping omitted stale, future, and guard-configuration rejection statuses.
11. Numeric scenario `ack_status` expectations were not normalized against the authoritative status map.
12. The top-level assurance runner could not select the native Pi build directory.

## 11. Repairs Made

The repair branch now:

- links the missing C++ sources;
- builds and registers all 18 tests;
- adds `astra_timing_campaign`;
- restores required configuration-service state;
- forwards app configuration through the executive;
- combines scheduler and app validation correctly;
- uses Unix/system wall-clock milliseconds for cross-machine command freshness;
- aligns simulated test command timestamps with mission timestamps;
- supports `--build-dir` in scenario, HIL, Monte Carlo, package, aggregate-verification, and assurance workflows;
- normalizes numeric acknowledgement expectations;
- includes all command-status values in the Python decoder;
- packages reproducibly from the explicitly selected Pi build directory.

## 12. Remaining Limitations and Claim Boundaries

The evidence does not establish:

- certification under any civil, military, or spaceflight standard;
- flight qualification;
- hard-real-time determinism or worst-case execution-time guarantees;
- airworthiness;
- production readiness;
- radiation tolerance;
- spacecraft bus, sensor, actuator, or payload compatibility;
- robustness to every malformed or adversarial input;
- correctness of requirements still marked `PLANNED`;
- automatic verification of requirements marked `MANUAL`;
- exact Raspberry Pi board/SoC model, which the restricted connector did not expose.

The timing campaign executes the software faster than simulated mission time and measures host execution cost. It does not schedule the process against real 10 ms wall-clock deadlines and must not be presented as a real-time qualification result.

## 13. Reproduction Entry Points

Native Pi build and tests:

```bash
cmake -S . -B build-pi -DCMAKE_BUILD_TYPE=Release
cmake --build build-pi --parallel
ctest --test-dir build-pi --output-on-failure
```

Complete assurance workflow using the native build:

```bash
python3 tools/run_astra_os_assurance.py --build-dir build-pi
```

Deterministic scenarios and checks without rebuilding:

```bash
python3 tools/run_all_scenarios.py \
  --build-dir build-pi \
  --skip-build \
  --skip-monte-carlo \
  --skip-pi-package
```

Seeded Monte Carlo regression:

```bash
python3 tools/run_monte_carlo.py \
  --build-dir build-pi \
  --trials 25 \
  --seed 20260626
```

Deployment package:

```bash
bash tools/package_pi_deployment.sh --build-dir build-pi
```

## 14. Final Disposition

The Raspberry Pi software, verification harness, and assurance workflow are suitable for a transparent educational portfolio demonstration with preserved, reproducible evidence. The branch is ready for review and pull-request integration after final documentation/evidence commits and remote push. Merge remains an explicit human approval action.
