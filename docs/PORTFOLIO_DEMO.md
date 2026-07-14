# ASTRA-OS 90-Second Portfolio Demo

This script is designed for a concise screen recording. It demonstrates the real repository and evidence without overstating certification or flight readiness.

## Preparation

From the repository root:

```bash
cmake -S . -B build-pi -DCMAKE_BUILD_TYPE=Release
cmake --build build-pi --parallel
```

Open the architecture image and final report in two browser tabs:

- `docs/assets/astra_os_architecture.svg`
- `reports/ASTRA_OS_FINAL_COMPLETION_REPORT.md`

## Recording sequence

### 0–15 seconds — architecture

Show the architecture diagram.

Narration:

> ASTRA-OS is a deterministic spacecraft-style flight-software and assurance platform written in C++17 and Python. Commands pass through protocol validation, replay and freshness protection, and a separate authorization policy before reaching the scheduled flight-software core.

### 15–40 seconds — deterministic behavior

Run:

```bash
python3 tools/run_scenario.py scenarios/recovery_failure_failsafe.yaml --build-dir build-pi
```

Narration:

> This scenario enters NOMINAL, injects a critical sensor fault, enters RECOVERY, and then makes three prohibited recovery-exit attempts. The recovery supervisor forces the system back to SAFE on the configured limit.

Expected final line:

```text
Scenario passed.
```

### 40–58 seconds — fault campaign

Run:

```bash
python3 tools/run_fdir_campaign.py --build-dir build-pi
```

Narration:

> The campaign exercises all ten supported fault dispositions through the real command and telemetry boundary and verifies each resulting mode.

Expected summary:

```text
FDIR cases: 10
[PASS] ...
```

### 58–75 seconds — complete verification

Show `reports/latest/assurance_summary.json` and `reports/requirement_check_report.md`.

Narration:

> The definitive campaign passed twenty native and sanitizer CTest suites, twenty-seven Python tests, eight deterministic scenarios, twenty-five seeded Monte Carlo trials, and a 129-case permission evaluation, with zero requirement failures or traceability problems.

### 75–90 seconds — Raspberry Pi and claim boundary

Show the `reports/pi-hil/` timing and resource evidence.

Narration:

> The same portable core was built and executed natively on Raspberry Pi, including a one-million-cycle soak. These are observed educational engineering results—not certification, hard-real-time proof, or flight qualification.

## Suggested title

```text
ASTRA-OS: Deterministic Flight Software, FDIR, and Raspberry Pi Assurance
```

## Suggested description

```text
A 90-second demonstration of ASTRA-OS, a C++17/Python spacecraft-style flight-software and assurance platform featuring deterministic scheduling, command/telemetry protocols, replay and freshness protection, authorization policy, ten-case FDIR, bounded recovery supervision, requirements traceability, sanitizer and mutation testing, Monte Carlo regression, and Raspberry Pi evidence.
```
