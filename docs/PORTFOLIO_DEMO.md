# ASTRA-OS 90-Second Portfolio Demo

This outline is designed for a concise screen recording of the repository and its evidence. Keep the distinction between the evolving current tree and frozen historical baselines visible throughout.

## Preparation

Build the portable software and open these artifacts:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DASTRA_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

- `docs/assets/astra_os_architecture.svg`
- `reports/ASTRA_OS_FINAL_COMPLETION_REPORT.md`
- `reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md`
- the GitHub Actions page for the current branch

## Suggested sequence

### 0–20 seconds — architecture

Show the architecture and explain that ASTRA-OS is a C++17/Python spacecraft-style flight-software and assurance platform with deterministic modes, command/telemetry, FDIR, bounded recovery, and configuration control.

### 20–45 seconds — deterministic software behavior

Show one deterministic scenario and the ten-case FDIR report. Emphasize that the scenarios exercise documented software policy through the command/telemetry boundary; they are not a substitute for representative spacecraft hardware integration.

### 45–70 seconds — assurance evidence

Show the current CI results and requirement report.

Current-tree summary:

```text
CTest suites:                    20/20
Python tool + browser tests:     35/35
Deterministic scenarios:          8/8
FDIR cases:                      10/10
Protocol checks:                 24/24
Seeded Monte Carlo:              25/25
Permission-boundary cases:      129/129
Requirement failures:                0
Traceability problems:               0
```

Explain that the frozen completion and v1.0.0 reports retain the smaller Python-test counts that existed at their exact tested commits. Historical evidence is intentionally not rewritten when the repository adds tests later.

### 70–90 seconds — native target evidence and boundary

Show the Raspberry Pi verification report and preserved `reports/pi-hil/` evidence.

Explain that the portable core was built and executed natively on Ubuntu 24.04 `aarch64` with a Raspberry Pi kernel, including timing and soak observations. The directory name is historical; the supported claim is native target execution, not representative spacecraft HIL, WCET proof, certification, or flight qualification.

## Suggested title

```text
ASTRA-OS: Deterministic Flight Software, FDIR, and Assurance Evidence
```

## Suggested description

```text
A concise walkthrough of ASTRA-OS, a C++17/Python spacecraft-style flight-software and assurance project with deterministic scheduling, command/telemetry, FDIR, bounded recovery, configuration control, requirements traceability, randomized and fault campaigns, and preserved native Raspberry Pi/aarch64 execution evidence.
```
