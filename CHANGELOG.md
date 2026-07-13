# Changelog

All notable ASTRA-OS changes are recorded here. Dates use UTC. The project does not claim semantic-version stability until the first ASTRA-OS release is tagged.

## Unreleased — ASTRA-OS assurance baseline

### Added

- `PROJECT_STATE.md` with verified baseline, backlog, risks, and continuation point.
- Draft pull-request workflow for independent branch validation.
- Configurable warnings-as-errors, AddressSanitizer, UndefinedBehaviorSanitizer, coverage, and clang-tidy build options.
- ASTRA-OS software-assurance workflow with sanitizer, coverage, static-analysis, Python, shell, and controlled-mutation jobs.
- Deterministic command malformed-input campaign covering every truncated packet length, oversized packets, invalid magic, invalid version, invalid command ID, failed-output preservation, and 208 single-bit corruptions.
- Wrap-aware duplicate and replay rejection for decoded ground commands.
- Deterministic tick-driven `RateGroupScheduler` with periods, phases, deadlines, discontinuity rejection, completion tracking, overruns, and statistics.
- Controlled CRC-rejection defect used to measure command-test effectiveness.
- Machine-readable provenance manifest generator with commit, dirty state, host, toolchain, build settings, commands, and SHA-256 input hashes.
- Canonical architecture, requirements, verification matrix, decisions, and risk register.
- CI upload of full verification reports and provenance artifacts.

### Changed

- Existing lowercase architecture and requirements paths now point to canonical uppercase living documents.
- Requirement checking now validates the canonical requirements baseline and verification matrix.
- Generated evidence ignore patterns are scoped so canonical CSV traceability files remain trackable.
- C++ extensions are disabled and compile command export is enabled.

### Preserved

- Existing command, telemetry, mode, fault, health, watchdog, scenario, Monte Carlo, deployment-package, and Raspberry Pi evidence paths.
- Existing mode and fault numeric values.
- Existing command packet size and version.
- Existing repository history and default branch.

### Evidence status

- Normal C++ CI has passed the command hardening increment.
- Scheduler, assurance workflow, mutation, provenance, and expanded traceability remain subject to the final pull-request checks before their requirement status is promoted from Implemented to Verified.

## AstraSim-FSW historical baseline

The predecessor baseline at commit `b63dad495ba921e855e21672831edee444502061` documented nine passing CTest suites, deterministic scenarios, command and telemetry demonstrations, Monte Carlo regression, deployment packaging, and Raspberry Pi execution evidence. Those records are preserved as historical evidence and are not automatically treated as current ASTRA-OS results.
