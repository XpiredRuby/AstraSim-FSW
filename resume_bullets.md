# ASTRA-OS Resume Bullets

Use only bullets that match the role and the evidence you are prepared to discuss in detail.

- Built ASTRA-OS, a C++17/Python spacecraft-style flight-software and assurance platform with deterministic scheduling, binary UDP command/telemetry, health/watchdog monitoring, explicit FDIR, bounded recovery, and configuration control.
- Implemented command integrity and execution controls including CRC-16-CCITT, duplicate/replay rejection, stale/future timestamp guards, typed acknowledgements, and a separate authorization-policy layer.
- Built an automated assurance campaign spanning **20/20 CTest suites, 8/8 deterministic scenarios, 10/10 fault cases, 24/24 protocol-conformance checks, 25/25 seeded Monte Carlo trials, and 129/129 frozen permission-boundary cases**, with zero requirement or traceability failures on the current baseline.
- Built and executed the portable core natively on Ubuntu 24.04 `aarch64` with a Raspberry Pi kernel, preserving target build/test, timing/soak, resource, and deployment-package evidence; the repository explicitly does not present this as spacecraft HIL, flight qualification, or hard-real-time proof.
- Added requirement-to-test traceability, controlled-interface hashes, provenance manifests, sanitizers, clang-tidy, coverage, bounded fuzzing, and a controlled mutation test that demonstrates the command tests reject a known CRC-acceptance defect.
