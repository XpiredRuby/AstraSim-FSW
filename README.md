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
