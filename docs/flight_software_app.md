# FlightSoftwareApp

`FlightSoftwareApp` is the central application layer that connects the flight software state machine, command processing, and telemetry generation.

## Purpose

Before this module, the project had separate pieces:

- `ModeManager`
- `CommandPacket`
- `CommandProcessor`
- `TelemetryPacket`
- UDP command and telemetry modules

`FlightSoftwareApp` provides a single core step function that can be used by future demos, simulations, Raspberry Pi deployment, and HIL tests.

## Step Input

Each app step receives:

- Optional decoded command
- Timestamp
- CPU load estimate
- Memory load estimate
- Heartbeat count

## Step Output

Each app step returns:

- Whether a command was processed
- The command result, if applicable
- A telemetry packet representing the current flight software state

## Current Behavior

- Starts in `BOOT`
- Processes valid commands through `CommandProcessor`
- Preserves mode and fault state internally
- Generates telemetry every step
- Increments telemetry sequence number automatically

## Future Use

Future modules should call `FlightSoftwareApp::step()` instead of manually wiring together:

- `ModeManager`
- `CommandProcessor`
- `TelemetryPacket`

This keeps the flight software behavior centralized and easier to test.
