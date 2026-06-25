#!/usr/bin/env bash
set -euo pipefail

# AstraSim-FSW docs repair/setup script.
# Run from project root:
#   cd ~/projects/AstraSim-FSW
#   bash apply_docs_step.sh

if [ ! -d "fsw" ] || [ ! -f "CMakeLists.txt" ]; then
  echo "ERROR: Run this from the AstraSim-FSW project root."
  exit 1
fi

mkdir -p docs

cat > README.md <<'EOF'
# AstraSim-FSW

AstraSim-FSW is a C++/Python flight-software-in-the-loop simulation and HIL verification framework.

The project uses a Raspberry Pi as an embedded flight-software target and a laptop as the simulation, test, telemetry, and fault-injection environment.

## Current Status

- C++ project skeleton created
- CMake build system working
- GitHub repository connected
- Flight software starts in BOOT mode
- Flight software transitions to NOMINAL mode
- ModeManager module added
- Simulated CPU overload fault transitions NOMINAL to DEGRADED_PAYLOAD

## Build and Run

```bash
mkdir -p build
cd build
cmake ..
make -j4
./astra_fsw
```

## Current Demo Output

```text
AstraSim-FSW starting...
Mode: BOOT
Mode transition: BOOT -> NOMINAL
Heartbeat 1 | Mode: NOMINAL
Heartbeat 2 | Mode: NOMINAL
Heartbeat 3 | Mode: NOMINAL
Heartbeat 4 | Mode: NOMINAL
Heartbeat 5 | Mode: NOMINAL
Injecting simulated CPU overload fault...
Mode transition: NOMINAL -> DEGRADED_PAYLOAD | Reason: CPU_OVERLOAD
AstraSim-FSW shutdown.
```

## Planned System

The final system will include:

- C++ embedded flight software core
- BOOT / NOMINAL / DEGRADED / SAFE / RECOVERY mode manager
- UDP telemetry from Raspberry Pi to laptop
- Command packets from laptop to Raspberry Pi
- Watchdog and health monitor
- Sensor and webcam/payload health checks
- Fault injection
- YAML scenario runner
- Requirement checker
- Monte Carlo regression runner
- HIL/SIL testing workflow
- GitHub and portfolio-ready documentation

## Project Goal

The goal is to build a professional-grade simulation and flight-software verification framework that demonstrates embedded C++, simulation infrastructure, telemetry, fault handling, and automated verification.
EOF

cat > docs/architecture.md <<'EOF'
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
EOF

cat > docs/requirements.md <<'EOF'
# Requirements

| ID | Requirement | Verification Method | Status |
|---|---|---|---|
| REQ-001 | Flight software shall start in BOOT mode | Run output / future unit test | Implemented |
| REQ-002 | Flight software shall transition to NOMINAL after initialization | Run output / future unit test | Implemented |
| REQ-003 | CPU overload shall transition system to DEGRADED_PAYLOAD | Fault test | Implemented as simulated fault |
| REQ-004 | Critical sensor failure shall transition system to SAFE mode | Future HIL/SIL test | Planned |
| REQ-005 | Flight software shall send telemetry to laptop | Future UDP test | Planned |
| REQ-006 | Flight software shall receive commands from laptop | Future UDP command test | Planned |
| REQ-007 | Corrupted commands shall be rejected | Future command parser test | Planned |
| REQ-008 | Requirement checker shall output PASS/FAIL results | Future scenario test | Planned |
EOF

cat > docs/test_plan.md <<'EOF'
# Test Plan

## Current Tests

1. Build the C++ flight software.
2. Run the executable.
3. Confirm BOOT mode appears.
4. Confirm BOOT to NOMINAL transition.
5. Confirm CPU overload fault transitions to DEGRADED_PAYLOAD.

## Manual Test Command

```bash
mkdir -p build
cd build
cmake ..
make -j4
./astra_fsw
```

## Expected Output

```text
AstraSim-FSW starting...
Mode: BOOT
Mode transition: BOOT -> NOMINAL
Heartbeat 1 | Mode: NOMINAL
Heartbeat 2 | Mode: NOMINAL
Heartbeat 3 | Mode: NOMINAL
Heartbeat 4 | Mode: NOMINAL
Heartbeat 5 | Mode: NOMINAL
Injecting simulated CPU overload fault...
Mode transition: NOMINAL -> DEGRADED_PAYLOAD | Reason: CPU_OVERLOAD
AstraSim-FSW shutdown.
```

## Future Tests

- UDP telemetry test
- Command parser test
- Watchdog timeout test
- Sensor fault test
- Payload fault test
- Monte Carlo scenario test
EOF

echo "Docs updated successfully."
echo
git status --short
