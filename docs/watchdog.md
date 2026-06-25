# Watchdog

`Watchdog` tracks flight-software loop timing and detects missed deadlines or stale execution.

## Purpose

A watchdog is a core flight-software safety mechanism.

It verifies that the flight software continues to run within expected timing limits. If the app stops checking in or repeatedly misses loop deadlines, the watchdog produces a fault.

## Inputs

`Watchdog::evaluate()` receives:

- Current time in milliseconds
- Current loop duration in milliseconds

## Outputs

`WatchdogReport` contains:

- Watchdog status
- Fault code
- Human-readable message
- Last kick time
- Elapsed time since last kick
- Missed deadline count
- Alive flag
- Loop timing flag

## Current Fault Mapping

| Condition | Fault |
|---|---|
| Watchdog timeout expired | `WATCHDOG_DEADLINE_MISS` |
| Maximum missed loop deadlines exceeded | `WATCHDOG_DEADLINE_MISS` |

## Severity Rules

- Normal timing produces `OK`.
- Warning threshold or a single loop deadline miss produces `WARNING`.
- Timeout expiration or repeated missed deadlines produces `EXPIRED`.

## Future Use

Future versions should connect `Watchdog` into `FlightSoftwareApp` so watchdog expiration automatically creates a critical health fault and drives the mode manager into a safe state.
