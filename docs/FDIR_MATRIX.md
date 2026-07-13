# ASTRA-OS FDIR Disposition Matrix

## Scope and claim boundary

This matrix distinguishes a **policy disposition** from an implemented detector and from end-to-end hardware validation. Each supported `FaultCode` now has a deterministic software disposition and command/telemetry campaign case. That does not prove the corresponding physical failure can be detected or recovered on spacecraft hardware.

| Fault | Detection or policy source | Detector status | Severity | Priority | Primary response | Persistence intent | Recovery rule | Verification evidence |
|---|---|---|---|---:|---|---|---|---|
| `SENSOR_TIMEOUT` | Health-monitor sensor age | Implemented in SIL | Critical | 90 | Enter SAFE | Latched fault state | Operator clear after valid-sensor review | Unit tests deterministic scenario and FDIR campaign |
| `SENSOR_INVALID_DATA` | Sensor-validity policy input | Policy injection path | Critical | 85 | Enter SAFE | Latched fault state | Operator clear after valid-sensor review | FDIR unit test and FDIR campaign |
| `PAYLOAD_HEARTBEAT_TIMEOUT` | Health-monitor payload age | Implemented in SIL | Degraded | 50 | Enter DEGRADED_PAYLOAD | Latched fault state | Clear after heartbeat recovery review | Health unit test and FDIR campaign |
| `CPU_OVERLOAD` | Resource monitor | Implemented in SIL | Degraded | 60 | Enter DEGRADED_PAYLOAD | Latched fault state | Clear after CPU recovery review | Unit scenario simultaneous-fault and FDIR campaign evidence |
| `MEMORY_OVERLOAD` | Resource monitor | Implemented in SIL | Critical | 80 | Enter SAFE | Latched fault state | Operator clear after memory recovery review | Health and FDIR unit tests plus FDIR campaign |
| `TELEMETRY_SOCKET_FAILURE` | Telemetry-transport policy input | Manual injection path | Critical | 75 | Enter SAFE | Latched fault state | Operator clear after transport recovery review | FDIR unit test and FDIR campaign |
| `COMMAND_BAD_CRC` | Command decoder policy mapping | Decoder rejects malformed packet | Advisory | 20 | Hold current mode | Not latched by policy response | Automatic after a valid packet | Parser mutation fuzz and FDIR campaign evidence |
| `COMMAND_UNKNOWN_ID` | Command decoder policy mapping | Decoder rejects unknown ID | Advisory | 15 | Hold current mode | Not latched by policy response | Automatic after a valid command | Parser fuzz and FDIR campaign evidence |
| `COMMAND_TIMEOUT` | Command-link policy input | Manual injection path | Critical | 70 | Enter SAFE | Latched fault state | Operator clear after link recovery review | FDIR unit test and FDIR campaign |
| `WATCHDOG_DEADLINE_MISS` | Watchdog | Implemented in SIL | Critical | 100 | Enter SAFE | Latched fault state | Operator clear after timing investigation | Unit deterministic simultaneous-fault and FDIR campaign evidence |

## Deterministic simultaneous-fault policy

1. Ignore `NONE` observations.
2. Select the active fault with the highest configured priority.
3. Break equal-priority ties using the lower numeric fault code.
4. Process only the selected internal fault during the current application cycle.
5. Preserve the health and watchdog reports so diagnostics retain non-selected observations.
6. Re-evaluate every application cycle.
7. If a degraded response is invalid from the current mode then fall back to SAFE.

The highest current priority is `WATCHDOG_DEADLINE_MISS` at 100. The policy is verified by `fdir_manager_tests` and `simultaneous_fault_tests`.

## Bounded recovery supervision

Entering `RECOVERY` starts a recovery-supervision session. A successful exit resets the failure count. Three consecutive prohibited recovery-exit commands force the mode back to SAFE and produce `REJECTED_RECOVERY_LIMIT` telemetry acknowledgement. Evidence:

- `recovery_supervisor_tests`
- `scenarios/recovery_failure_failsafe.yaml`
- `reports/scenario_recovery_failure_failsafe_output.txt`

## Ten-case campaign

`tools/run_fdir_campaign.py` injects every supported fault through the real UDP command/telemetry scenario boundary and verifies the acknowledgement fault indication and resulting mode. The committed result is:

```text
10/10 cases passed
```

Primary report: `reports/fdir_campaign_report.md`.

## Remaining physical-system limitations

The following are outside the current software-only claim:

- physical sensor disagreement and actuator-nonresponse detection;
- automatic command-link and telemetry-transport fault sensing on representative spacecraft interfaces;
- radiation and single-event-effect behavior;
- full active-fault-set persistence beyond the selected primary fault;
- hardware recovery dwell tuning derived from mission requirements;
- target-specific process restart and power-cycle recovery.

These limitations do not invalidate the verified software policy paths but they prevent flight-qualification or spacecraft-hardware-compatibility claims.
