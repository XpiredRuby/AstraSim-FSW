# ASTRA-OS Project State

## Current status

The current development baseline is the repository default branch, `main`. The completed v1.0.0 software/assurance baseline and earlier native Raspberry Pi campaigns remain preserved through tags, reports, and provenance artifacts.

ASTRA-OS is an educational spacecraft-style flight-software and assurance platform. The portable software baseline has no known open implementation blocker, but that statement is deliberately narrower than flight readiness: representative spacecraft hardware integration, mission-derived timing budgets, cryptographic commanding, radiation behavior, and certification are outside the demonstrated scope.

## Implemented software scope

- eight operational modes including STANDBY and TEST;
- deterministic mode-transition rejection;
- binary command and telemetry protocols with CRC;
- duplicate/replay, stale, and future command rejection;
- configurable command-execution authorization policy;
- health and watchdog monitoring;
- ten-fault FDIR disposition table with deterministic simultaneous-fault priority;
- bounded recovery supervision that forces SAFE after repeated failed exits;
- deterministic rate-group scheduling and executive dispatch;
- versioned configuration validation, revision control, and runtime lock;
- bounded typed event logging;
- UDP demonstrations and Python ground tooling;
- deterministic scenarios, seeded Monte Carlo, and ten-case FDIR campaign;
- native Raspberry Pi/aarch64 execution and deployment-package evidence;
- protocol conformance, requirement traceability, reverse CTest allocation, and reviewed interface hashes;
- sanitizer, clang-tidy, coverage, mutation, fuzzing, timing/soak, and provenance workflows;
- deterministic governed-assurance permission interface with frozen quantitative evaluation.

## Current repository verification snapshot

The evolving `main` branch currently carries:

```text
Native CTest:                     20/20
Python tool + browser-model tests: 35/35
Declared YAML scenarios:          8/8
Ten-case FDIR campaign:           10/10
Protocol conformance:             24/24
Seeded Monte Carlo:               25/25
Assurance-assistant evaluation:   129/129
Requirement failures:             0
Traceability problems:             0
Planned canonical requirements:    0
```

Historical reports retain the exact counts from their own frozen commits. For example, the final-completion report records 27 Python tooling tests and the v1.0.0 release notes record 28; later browser-model/tool tests raised the current-tree count to 35. Those older reports are evidence of their tested baselines and are intentionally not rewritten.

## Evidence locations

- `reports/ASTRA_OS_FINAL_COMPLETION_REPORT.md`
- `reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md`
- `docs/REQUIREMENTS.md`
- `docs/VERIFICATION_MATRIX.csv`
- `config/traceability_baseline.json`
- `reports/requirement_check_report.md`
- `reports/fdir_campaign_report.md`
- `reports/assurance_assistant_eval.md`
- `reports/monte_carlo_report.md`
- `reports/pi_deployment_package_report.md`
- `reports/pi-hil/` (historical target-evidence directory name)
- `reports/latest/`

## Completion definition

Portable software completion means:

- canonical requirements are implemented or explicitly bounded by the project claim;
- registered tests are allocated to requirements;
- deterministic and randomized verification gates pass;
- the native package can be reproduced;
- final assurance evidence is tied to an exact source state;
- documentation distinguishes current-tree results from frozen historical baselines.

It does **not** mean certification, flight qualification, spacecraft-hardware compatibility, production security, operational reliability, or hard-real-time qualification.

## Remaining external work

Future expansion requires representative mission or hardware authority for items such as:

- cryptographic command authentication and key management;
- spacecraft buses, sensors, actuators, payload and power interfaces;
- radiation and single-event-effect behavior;
- mission-derived scheduling budgets and WCET evidence;
- physical detector calibration and recovery dwell;
- certification and operational approval.
