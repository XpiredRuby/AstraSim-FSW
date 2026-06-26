# Monte Carlo Regression

AstraSim-FSW includes a Monte Carlo regression runner that repeatedly generates randomized command/fault verification scenarios and executes them through the same command/telemetry scenario harness used by the deterministic YAML tests.

## Command

```bash
python3 tools/run_monte_carlo.py --trials 25
```

For reproducible results, pass a fixed seed:

```bash
python3 tools/run_monte_carlo.py --trials 25 --seed 20260626
```

## Covered behavior

The randomized trial set exercises:

- valid BOOT to NOMINAL command transitions
- invalid transition rejection
- CPU overload fault handling
- fault clearing
- sensor timeout transition to SAFE mode
- watchdog deadline miss transition to SAFE mode
- telemetry command acknowledgement fields

## Evidence

The runner writes a summary report to:

```text
reports/monte_carlo_report.md
```
