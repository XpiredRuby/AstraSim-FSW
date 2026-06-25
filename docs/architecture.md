# AstraSim-FSW Architecture

AstraSim-FSW is organized as a flight-software core plus Python ground tooling.

The C++ side models the embedded flight-software target. The Python side models ground tools for command injection, telemetry reception, and automated demo capture.

## High-Level Architecture

```text
+-----------------------------+
| Python Ground Tools         |
|                             |
|  send_command.py            |
|  telemetry_receiver.py      |
|  demo capture scripts       |
+--------------+--------------+
               |
               | UDP command packets
               v
+--------------+--------------+
| C++ Flight Software Target  |
|                             |
|  UdpCommandReceiver         |
|  CommandPacket              |
|  FlightSoftwareApp          |
|  Watchdog                   |
|  HealthMonitor              |
|  CommandProcessor           |
|  ModeManager                |
|  TelemetryPacket            |
|  UdpTelemetrySender         |
+--------------+--------------+
               |
               | UDP telemetry packets
               v
+--------------+--------------+
| Python Ground Telemetry     |
+-----------------------------+
```

## FlightSoftwareApp

`FlightSoftwareApp` is the central application layer.

It owns and connects:

- `ModeManager`
- `CommandProcessor`
- `HealthMonitor`
- `Watchdog`
- Telemetry generation

Future demos and deployment targets should call `FlightSoftwareApp::step()` instead of manually wiring the modules together.

## Command Path

```text
Python command sender
        |
        v
UDP datagram
        |
        v
UdpCommandReceiver
        |
        v
CommandPacket decode + CRC validation
        |
        v
FlightSoftwareApp
        |
        v
CommandProcessor
        |
        v
ModeManager
```

## Health and Watchdog Path

```text
FlightSoftwareStepInput
        |
        +--> HealthMonitor
        |
        +--> Watchdog
        |
        v
Automatic internal INJECT_FAULT command
        |
        v
CommandProcessor
        |
        v
ModeManager
```

## Telemetry Path

```text
FlightSoftwareApp state
        |
        v
TelemetryPacket encode + CRC
        |
        v
UdpTelemetrySender
        |
        v
Python telemetry_receiver.py
```

## Current Safety Behavior

- CPU overload can drive `NOMINAL -> DEGRADED_PAYLOAD`.
- Sensor timeout can drive `NOMINAL -> SAFE`.
- Watchdog expiration can drive `NOMINAL -> SAFE`.
- Warning-level health/watchdog reports do not automatically inject faults.
- Critical health reports and expired watchdog reports are converted into internal `INJECT_FAULT` commands.

## Test Strategy

The project currently uses small C++ executable test suites registered with CTest.

Current test suites:

- Mode manager
- Telemetry packet
- UDP telemetry sender
- Command packet
- Command processor
- UDP command receiver
- Flight software app
- Health monitor
- Watchdog

The CI workflow builds the project and runs the local test script.
