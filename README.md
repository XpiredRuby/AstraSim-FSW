# AstraSim-FSW

[![C++ Build and Unit Tests](https://github.com/XpiredRuby/AstraSim-FSW/actions/workflows/unit_tests.yml/badge.svg)](https://github.com/XpiredRuby/AstraSim-FSW/actions/workflows/unit_tests.yml)

AstraSim-FSW is a C++/Python flight-software-in-the-loop simulation and verification framework.

The project uses C++ for the embedded flight-software core and Python for ground-side tooling, command injection, telemetry decoding, and repeatable demo capture. The intended hardware path is a Raspberry Pi acting as the embedded flight-software target and a laptop acting as the simulation, command, telemetry, and verification environment.

## Current Status

Implemented:

- C++17 flight software core
- CMake build system
- GitHub Actions CI
- BOOT / NOMINAL / DEGRADED / SAFE / RECOVERY mode manager
- Fault-driven mode transitions
- Binary telemetry packet serialization
- CRC-16-CCITT telemetry validation
- UDP telemetry sender
- Python telemetry receiver
- Binary command packet serialization
- CRC-16-CCITT command validation
- UDP command receiver
- Python command sender
- Command processor connected to the mode manager
- Live UDP telemetry demo
- Integrated command/telemetry demo
- Automated demo capture reports

Current test coverage:

- Mode manager tests
- Telemetry packet tests
- UDP telemetry sender tests
- Command packet tests
- Command processor tests
- UDP command receiver tests

## System Flow

```text
Python command sender
        |
        v
UDP command packet
        |
        v
C++ UDP command receiver
        |
        v
CommandPacket decoder + CRC validation
        |
        v
CommandProcessor
        |
        v
ModeManager
        |
        v
TelemetryPacket encoder + CRC
        |
        v
C++ UDP telemetry sender
        |
        v
Python telemetry receiver
```

## Build and Test

From the project root:

```bash
bash ci/run_local_tests.sh
```

Manual build:

```bash
rm -rf build
mkdir -p build
cd build
cmake ..
make -j4
ctest --output-on-failure
```

## Main Executables

| Executable | Purpose |
|---|---|
| `astra_fsw` | Basic flight software mode/fault demo |
| `astra_fsw_telemetry_demo` | Sends live UDP telemetry to the Python receiver |
| `astra_fsw_command_telemetry_demo` | Receives UDP commands, updates mode/fault state, and sends UDP telemetry |

## Python Tools

| Tool | Purpose |
|---|---|
| `tools/telemetry_receiver.py` | Receives and decodes AstraSim-FSW telemetry packets |
| `tools/send_command.py` | Sends binary UDP command packets |
| `tools/run_live_telemetry_demo.sh` | Automates telemetry sender/receiver demo capture |
| `tools/run_command_telemetry_demo.sh` | Automates full command/telemetry demo capture |

## Live Telemetry Demo

Run:

```bash
bash ci/run_local_tests.sh
tools/run_live_telemetry_demo.sh
```

This writes:

```text
reports/live_telemetry_demo_output.txt
```

## Command / Telemetry Demo

Run:

```bash
bash ci/run_local_tests.sh
tools/run_command_telemetry_demo.sh
```

This writes:

```text
reports/command_telemetry_demo_output.txt
```

Expected command sequence:

```text
SET_MODE NOMINAL
INJECT_FAULT CPU_OVERLOAD
CLEAR_FAULT
```

Expected system behavior:

```text
BOOT -> NOMINAL
NOMINAL -> DEGRADED_PAYLOAD after CPU_OVERLOAD
Fault state clears after CLEAR_FAULT
```

## Documentation

| Document | Purpose |
|---|---|
| `docs/architecture.md` | System architecture overview |
| `docs/requirements.md` | Initial requirements table |
| `docs/test_plan.md` | Test plan |
| `docs/live_telemetry_demo.md` | Live telemetry demo instructions |
| `docs/command_packet.md` | Command packet format |
| `docs/command_processor.md` | Command processor behavior |
| `docs/udp_command_receiver.md` | UDP command receiver design |
| `docs/command_telemetry_demo.md` | Integrated command/telemetry demo |

## Project Goal

The goal is to build a professional-grade simulation and flight-software verification framework that demonstrates:

- Embedded C++
- State-machine design
- Fault handling
- Binary packet design
- UDP command and telemetry links
- Python ground tooling
- Automated verification
- Repeatable demo evidence
- GitHub Actions CI

## Planned Next Steps

- Add a `FlightSoftwareApp` core loop
- Add command acknowledgement telemetry
- Add watchdog and health monitor
- Add YAML scenario runner
- Add requirement checker
- Add Monte Carlo regression runner
- Deploy the C++ target on Raspberry Pi
- Connect real sensor/payload health inputs
