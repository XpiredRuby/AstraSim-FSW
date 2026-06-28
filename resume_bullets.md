# AstraSim-FSW Resume Bullets

- Built AstraSim-FSW, a C++17/Python flight-software-in-the-loop verification framework for spacecraft-style mode management, UDP command/telemetry, fault injection, watchdog behavior, and Raspberry Pi HIL execution.
- Implemented binary command and telemetry packets with CRC-16-CCITT validation, UDP command receiver/sender tooling, telemetry decoding, and a live terminal dashboard for real-time mode/fault monitoring.
- Added automated verification with 9/9 CTest suites passing, 5/5 YAML scenarios passing, 25/25 Monte Carlo regression trials passing, requirement traceability reports, and committed demo evidence.
- Deployed and built the flight software on Raspberry Pi Ubuntu aarch64, then captured HIL evidence showing command acceptance, CPU fault injection, degraded-mode transition, fault clear, and recovery to NOMINAL.
