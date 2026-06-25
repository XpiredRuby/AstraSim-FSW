# Architecture

This document describes the AstraSim-FSW system architecture.

## Main Components

- Raspberry Pi embedded flight-software target
- Laptop simulation and test environment
- UDP telemetry link
- UDP command link
- Fault injection tools
- Requirement checker
- Monte Carlo regression runner

## Current Architecture

```text
Laptop / WSL
  |
  | future UDP command and telemetry link
  |
Raspberry Pi / embedded target
  |
  C++ flight software core
```

## Current Software Modules

- `main.cpp`: starts the flight software demo
- `ModeManager`: controls BOOT, NOMINAL, DEGRADED, SAFE, and RECOVERY modes

## Future Software Modules

- Telemetry packet encoder
- UDP telemetry sender
- Command parser
- Watchdog
- Health monitor
- Fault manager
- Scenario runner
- Requirement checker
