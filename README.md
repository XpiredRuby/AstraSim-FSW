# ASTRA-OS

**Spacecraft-style flight software with the assurance campaign that proves it behaves.** C++17 flight core, Python ground tooling, binary command and telemetry over UDP, FDIR, and a verification chain that runs on every commit.

[![Build and Unit Tests](https://github.com/XpiredRuby/AstraSim-FSW/actions/workflows/unit_tests.yml/badge.svg)](https://github.com/XpiredRuby/AstraSim-FSW/actions/workflows/unit_tests.yml)
[![Interactive console](https://img.shields.io/badge/interactive-mission%20console-5ee7ff)](https://xpiredruby.github.io/AstraSim-FSW/)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C)
![Python](https://img.shields.io/badge/Python-tooling-3776AB)
![Target](https://img.shields.io/badge/target-Raspberry%20Pi-C51A4A)

Flight software is judged on evidence, not features. The interesting question is never "does it work" but "show me the run where it did not, and show me what it did about it." ASTRA-OS is built around that question: deterministic operating modes, command integrity and authorization policy, health and watchdog monitoring, fault detection isolation and recovery, bounded recovery supervision, and a digital thread that ties every requirement to a test that actually executes.

**[Launch the interactive mission console](https://xpiredruby.github.io/AstraSim-FSW/)** and drive the state machine yourself: send mode commands, inject faults, trip the freshness and sequence guards, watch telemetry and event history respond.

![ASTRA-OS architecture](docs/assets/astra_os_architecture.svg)

---

## Verification results

Full managed assurance campaign, `overall_status: passed`.

| Area | Result |
|---|---:|
| Native CTest suites | 20 / 20 |
| Deterministic scenarios | 8 / 8 |
| FDIR fault campaign | 10 / 10 |
| Protocol conformance | 24 / 24 |
| Seeded Monte Carlo (seed `20260626`) | 25 / 25 |
| Python tool and web model tests | 35 / 35 |
| Frozen assurance-assistant evaluation | 129 / 129 |
| Requirement failures | 0 |
| Traceability problems | 0 |

| Mode timeline | Fault timeline |
|---|---|
| ![Mode timeline](media/plots/mode_timeline.svg) | ![Fault timeline](media/plots/fault_timeline.svg) |

A prior Raspberry Pi release-closure campaign is preserved in [`reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md`](reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md), verifying the predecessor baseline across 18 CTest suites and five deterministic scenarios.

## What it does

**Flight software core.** Eight deterministic modes (BOOT, NOMINAL, DEGRADED_SENSOR, DEGRADED_PAYLOAD, SAFE, RECOVERY, STANDBY, TEST) with validated transitions, health and watchdog reporting, ten-fault FDIR disposition with simultaneous-fault priority, bounded recovery supervision, a rate-group scheduler, versioned configuration control, and bounded typed event logging.

**Command and telemetry.** Fixed binary packet layouts with CRC-16-CCITT, a UDP command receiver and telemetry sender, Python sender and decoder, rejection of duplicate, replayed, stale and future-dated commands, a configurable execution authorization policy, and typed acknowledgements covering unauthorized and recovery-limit rejections. C++, Python and the protocol manifest are checked against each other for consistency.

CRC detects corruption under the tested model. It does not authenticate a sender, and `CommandAuthorizer` is execution policy rather than cryptographic identity.

**Verification and assurance.** 20 native CTest suites, eight deterministic YAML scenarios, a ten-case FDIR command and telemetry campaign, seeded Monte Carlo regression, ASan and UBSan, clang-tidy, aggregate and per-module LCOV coverage, controlled CRC mutation testing, a bounded libFuzzer campaign with deterministic seed corpus, timing and resource evidence, deployment packaging, provenance manifests, reverse CTest-to-requirement allocation, reviewed requirement fingerprints, controlled-interface hashes, and a deterministic governed-assistant policy with 129 frozen cases.

## How a command flows

```text
Python sender or YAML scenario
        │
        ▼  UDP command packet
CommandPacket decode + CRC-16-CCITT
        │
        ▼
GroundCommandGuard          timestamp age, future skew,
                            duplicate replay, wrap policy
        │
        ▼
CommandAuthorizer           configurable command and TEST policy
        │
        ▼
CommandProcessor            semantic validation
        │                   RecoverySupervisor, FDIRManager
        ▼
ModeManager + active fault

HealthMonitor ──┐
Watchdog     ───┴──►  internal fault selection

FlightSoftwareExecutive     RateGroupScheduler, app, housekeeping
        │
        ▼
TelemetryPacket + EventLogger
        │
        ▼  UDP telemetry
evidence reports
```

## Run it

Build and test:

```bash
cmake -S . -B build-pi -DCMAKE_BUILD_TYPE=Release -DASTRA_WARNINGS_AS_ERRORS=ON
cmake --build build-pi --parallel
ctest --test-dir build-pi --output-on-failure    # expect 20/20
```

Full assurance campaign, writing machine-readable status to `reports/latest/assurance_summary.json`:

```bash
python3 tools/run_astra_os_assurance.py --build-dir build-pi
```

Individual gates:

```bash
python3 tools/run_all_scenarios.py --build-dir build-pi --skip-build --skip-monte-carlo --skip-pi-package
python3 tools/run_fdir_campaign.py --build-dir build-pi
python3 tools/run_monte_carlo.py --build-dir build-pi --trials 25 --seed 20260626
python3 tools/check_requirements.py          # digital-thread and reverse-test gate
```

Package for the target:

```bash
bash tools/package_pi_deployment.sh --build-dir build-pi
```

## Scope

**Demonstrated:** a deterministic mode and FDIR state machine exercised through a real command and telemetry boundary, integrity and freshness enforcement on every command, bounded recovery with a supervised failure path, reproducible verification with sanitizers, mutation and fuzzing, requirement-to-test traceability enforced by a gate rather than asserted, and preserved Raspberry Pi execution evidence.

**Not established:** certification or DO-178C compliance, flight qualification or airworthiness, hard-real-time or worst-case execution time guarantees, production readiness or operational reliability, cryptographic command authentication, radiation tolerance, or compatibility with spacecraft hardware.

Raspberry Pi timing campaigns are documented host execution measurements, not real-time qualification. The browser console models the documented state machine; it is not the native executable, a WebAssembly build, or the Pi process.

## Stack

`C++17` `CMake` `CTest` `Python` `UDP` `CRC-16-CCITT` `YAML` `GitHub Actions` `Raspberry Pi`

Flight software architecture · finite state machines · fault detection isolation and recovery · watchdog and health monitoring · binary protocol design · command and telemetry · verification and validation · requirements traceability · Monte Carlo regression · fuzzing and mutation testing · sanitizers · static analysis · code coverage · provenance and configuration control

## Evidence

| Report | What it covers |
|---|---|
| [Final completion report](reports/ASTRA_OS_FINAL_COMPLETION_REPORT.md) | Campaign provenance, results, timing, resources, limitations |
| [Raspberry Pi verification](reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md) | Target execution evidence and defects |
| [Requirement check](reports/requirement_check_report.md) | Canonical requirement and digital-thread result |
| [FDIR campaign](reports/fdir_campaign_report.md) | Ten independently injected fault dispositions |
| [Assurance assistant evaluation](reports/assurance_assistant_eval.md) | 129-case permission evaluation |
| [Monte Carlo](reports/monte_carlo_report.md) | Seeded randomized regression |
| [Deployment package](reports/pi_deployment_package_report.md) | Target bundle contents and checksum |
| [`reports/latest/`](reports/latest/) | Protocol, sanitizer, mutation and provenance artifacts |

## Documentation

[Architecture](docs/ARCHITECTURE.md) · [Requirements](docs/REQUIREMENTS.md) · [Verification matrix](docs/VERIFICATION_MATRIX.csv) · [Assurance](docs/ASSURANCE.md) · [FDIR matrix](docs/FDIR_MATRIX.md) · [Command authorization](docs/command_authorization.md) · [Recovery supervisor](docs/recovery_supervisor.md) · [Risks and blockers](docs/RISKS_AND_BLOCKERS.md)

Case study: [`docs/PORTFOLIO_CASE_STUDY.md`](docs/PORTFOLIO_CASE_STUDY.md) · Release notes: [`RELEASE_NOTES_v1.0.0.md`](RELEASE_NOTES_v1.0.0.md)

## Deterministic scenarios

`basic_command_fault` · `command_timestamp_guard` · `extended_modes` · `hil_smoke_test` · `invalid_transition_rejected` · `recovery_failure_failsafe` · `sensor_timeout_safe_mode` · `watchdog_timeout_safe_mode`

## Governed assurance assistant

A policy-bounded interface for running verification targets. It cannot merge, push, run arbitrary shell commands, issue hardware commands, write project files, delete data, or mark requirements verified.

```bash
python3 tools/assurance_assistant.py --request '{"action":"run","target":"check_protocol_conformance"}'
python3 tools/run_assurance_assistant_eval.py       # frozen 129-case evaluation
```

## Author

**Vinayak Manoj Nair** · B.S. Aerospace Engineering, Texas A&M University

This is an educational portfolio project. Technical review and reproducibility feedback are welcome through GitHub issues.
