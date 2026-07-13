# Recovery Supervisor

## Purpose

`RecoverySupervisor` prevents unlimited unsuccessful attempts to leave `RECOVERY`. The default policy allows two failed exit attempts to remain in `RECOVERY`; the third consecutive failed exit forces SAFE.

## State behavior

- `SAFE -> RECOVERY` starts a new supervision session and clears the failure counter.
- A valid `RECOVERY -> STANDBY` or `RECOVERY -> NOMINAL` transition records success and clears the counter.
- A prohibited exit from `RECOVERY` increments the counter.
- When the configured maximum is reached the supervisor requests `RECOVERY -> SAFE` and the command acknowledgement becomes `REJECTED_RECOVERY_LIMIT`.
- Configuration value zero is invalid and prevents the application configuration from becoming valid.

The supervisor counts failed commanded recovery exits. It is not a mission-specific recovery dwell timer and it does not determine whether physical hardware has recovered.

## Configuration

```text
FlightSoftwareAppConfig.recovery_supervisor.maximum_failed_attempts
```

Default: `3`.

## Evidence

- `recovery_supervisor_tests`
- `configuration_service_tests`
- `scenarios/recovery_failure_failsafe.yaml`
- `reports/scenario_recovery_failure_failsafe_output.txt`

## Claim boundary

The bounded fallback demonstrates deterministic fail-safe software behavior. It does not establish fault containment time mission suitability hardware restart capability or flight qualification.
