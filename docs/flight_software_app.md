# FlightSoftwareApp

`FlightSoftwareApp` is the central application layer that connects the flight software state machine, command processing, health monitoring, and telemetry generation.

## Purpose

Before this module, the project had separate pieces:

- `ModeManager`
- `CommandPacket`
- `CommandProcessor`
- `TelemetryPacket`
- `HealthMonitor`
- UDP command and telemetry modules

`FlightSoftwareApp` provides a single core step function that can be used by future demos, simulations, Raspberry Pi deployment, and HIL tests.

## Step Input

Each app step receives:

- Optional decoded command
- Timestamp
- CPU load estimate
- Memory load estimate
- Heartbeat count
- Sensor data age
- Payload heartbeat age
- Loop duration

## Step Output

Each app step returns:

- Whether a ground command was processed
- The ground command result, if applicable
- The health report for the current step
- Whether an automatic health fault was processed
- The automatic health fault result, if applicable
- A telemetry packet representing the current flight software state

## Current Behavior

- Starts in `BOOT`
- Processes valid ground commands through `CommandProcessor`
- Evaluates health inputs through `HealthMonitor`
- Automatically injects `CRITICAL` health faults
- Does not inject faults for `WARNING` health reports
- Preserves mode and fault state internally
- Generates telemetry every step
- Increments telemetry sequence number automatically

## Fault Handling

Health faults are converted into internal `INJECT_FAULT` commands.

This keeps manual ground fault injection and automatic health fault detection on the same command-processing path.

## Future Use

Future modules should call `FlightSoftwareApp::step()` instead of manually wiring together:

- `ModeManager`
- `CommandProcessor`
- `HealthMonitor`
- `TelemetryPacket`

This keeps the flight software behavior centralized and easier to test.
