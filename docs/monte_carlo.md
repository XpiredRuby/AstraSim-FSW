# Monte Carlo Regression

ASTRA-OS includes a Monte Carlo regression runner that generates randomized command/fault scenarios and executes them through the same command/telemetry harness used by the deterministic YAML tests.

## Reproducible Raspberry Pi command

```bash
python3 tools/run_monte_carlo.py \
  --build-dir build-pi \
  --trials 25 \
  --seed 20260626
```

`--build-dir` may be relative to the repository root or absolute. The explicit seed is required for a reproducible release claim.

## Covered behavior

The randomized trial set exercises:

- valid BOOT-to-NOMINAL transitions;
- invalid transition rejection;
- CPU-overload fault handling;
- fault clearing;
- sensor-timeout transition to SAFE mode;
- watchdog-deadline-miss transition to SAFE mode;
- telemetry command acknowledgement fields;
- randomized UDP ports;
- randomized timeout trigger locations.

## Evidence

The runner writes a summary report to:

```text
reports/monte_carlo_report.md
```

The final Raspberry Pi campaign passed 25/25 trials with seed `20260626`. The result applies only to the executed trial count, seed, software commit, platform, and configuration; it is not exhaustive fault-space proof.
