# ASTRA-OS Software Architecture

## 1. Purpose

ASTRA-OS evolves the existing AstraSim-FSW codebase into a deterministic, testable flight-software and assurance platform. The architecture preserves the working command, telemetry, health, watchdog, scenario, and Raspberry Pi demonstration paths while progressively introducing explicit scheduling, traceability, stronger command semantics, and quantitative assurance evidence.

This is an educational and portfolio architecture. It does not represent a certified flight computer or a production avionics security design.

## 2. Architectural principles

1. Preserve externally observable working behavior unless a requirement authorizes a change.
2. Keep the flight-software core independent from UDP, files, Python tooling, and target hardware wherever practical.
3. Prefer deterministic state transitions and explicit inputs over hidden clocks, globals, or background threads.
4. Reject malformed or stale external inputs before they reach state-changing logic.
5. Keep internal fault injection distinguishable from ground command processing.
6. Treat evidence as a generated product with exact build and source provenance.
7. Add assurance mechanisms behind explicit build options so normal and instrumented builds remain reproducible.
8. Do not use AI output as numerical truth or as an automatic verification disposition.

## 3. Current component view

```text
Ground command tool
    |
    v
UDP command receiver
    |
    v
Command packet decoder
  - fixed length
  - CRC-16-CCITT
  - magic/version checks
  - command-ID validation
    |
    v
FlightSoftwareApp ground boundary
  - duplicate/replay sequence guard
  - command acknowledgement state
    |
    v
CommandProcessor ----------------------+
    |                                  |
    v                                  |
ModeManager                            |
                                       |
HealthMonitor ---- internal fault -----+
Watchdog --------- internal fault -----+

FlightSoftwareApp
    |
    v
Telemetry packet encoder
    |
    v
UDP telemetry sender --> ground receiver/dashboard
```

The deterministic `RateGroupScheduler` is currently a separately tested core service. It is not yet the production driver for `FlightSoftwareApp`; that integration is a planned, requirement-controlled change.

## 4. Core components

### 4.1 `ModeManager`

Owns the current operational mode and validates transitions. Existing mode numeric values are part of the command and telemetry interface and must remain stable. The current implementation supports BOOT, NOMINAL, DEGRADED_SENSOR, DEGRADED_PAYLOAD, SAFE, and RECOVERY. STANDBY and TEST are required additions but are not yet implemented.

### 4.2 `CommandPacket`

Defines the current 26-byte little-endian command format:

- 32-bit magic;
- 16-bit protocol version;
- 32-bit sequence number;
- 64-bit timestamp in milliseconds;
- 16-bit command ID;
- 32-bit argument;
- 16-bit CRC-16-CCITT.

Deserialization performs structural and integrity validation. Semantic validation of mode and fault arguments remains in the command-processing layer.

### 4.3 Ground command sequence guard

`FlightSoftwareApp` enforces replay protection after successful packet decoding and before command execution. It uses unsigned half-range serial-number arithmetic:

- the first syntactically valid ground command initializes the sequence state;
- an equal sequence is rejected as a duplicate;
- a sequence outside the forward half-range is rejected as replayed or stale;
- maximum-to-zero wrap is accepted;
- a semantically rejected command still consumes its sequence number.

This is replay resistance, not authentication. CRC protects accidental corruption and does not establish command authority.

### 4.4 `CommandProcessor`

Maps accepted commands to deterministic mode or fault operations and returns a structured disposition. It owns the last active fault state. It does not perform transport decoding or ground replay checks.

### 4.5 `HealthMonitor`

Evaluates an explicit snapshot containing CPU load, memory load, sensor age, payload age, and loop duration. It returns a status and fault recommendation. Critical recommendations are converted to internal fault commands by `FlightSoftwareApp`.

### 4.6 `Watchdog`

Evaluates explicit timestamps and loop duration. It distinguishes nominal, warning, and expired states. Expired watchdog outputs are converted to internal fault commands.

### 4.7 Internal fault path

Health and watchdog faults are created inside `FlightSoftwareApp` and sent directly to `CommandProcessor`. They intentionally bypass the ground sequence guard because they are not ground commands. Future telemetry and event logging must preserve the origin distinction.

### 4.8 `RateGroupScheduler`

A pure, tick-driven scheduler service with no threads, sleeps, or system-clock reads. Configuration defines task ID, name, period, phase, and relative deadline. The scheduler:

- rejects invalid configuration;
- requires monotonic contiguous ticks beginning at zero;
- emits deterministic release records in configuration order;
- tracks pending jobs, completions, overruns, and deadline misses;
- rejects discontinuities without advancing scheduler state;
- exposes cumulative statistics.

The scheduler does not execute callbacks. The application layer owns task dispatch and measured execution duration, which keeps scheduler decisions independently testable.

### 4.9 Telemetry

Telemetry includes software state, resource values, heartbeat, and the last attempted ground-command acknowledgement. The acknowledgement represents accepted and rejected attempts. Future revisions must add explicit validity and interface-revision fields without silently changing the current wire format.

## 5. Execution and concurrency model

The current core is single-threaded and step-driven. All state changes occur during explicit method calls. UDP demo processes and Python ground tools are external to the core. No service may introduce a hidden worker thread without an architecture decision, ownership model, shutdown behavior, and deterministic test seam.

The target scheduler integration will use a cyclic executive or rate-group dispatcher. Target-specific priority and real-time policy are intentionally outside the portable core until SIL behavior is stable.

## 6. Clock model

Current services receive time as integer milliseconds or scheduler ticks. The portable core shall not call the wall clock directly. A future time service will define:

- monotonic mission time;
- UTC correlation when available;
- clock validity;
- discontinuity handling;
- commanded or simulated clock changes;
- timestamp freshness tolerances.

Tests and scenarios shall use fake time sources.

## 7. Interface boundaries

### 7.1 Core boundary

The C++ core accepts typed structures and returns typed results. It shall not depend on sockets, YAML, command-line parsing, or report formatting for its decision logic.

### 7.2 Transport boundary

UDP receivers and senders own socket lifecycle, packet acquisition, and transport errors. Decoders own byte validation. State-changing services only receive decoded packets.

### 7.3 Simulation boundary

Python scenarios and future simulation bridges provide explicit snapshots, commands, delays, dropouts, and faults. Randomized campaigns shall publish their seeds and configuration.

### 7.4 Target boundary

Raspberry Pi deployment shall use the same core library and test vectors. Target-specific differences must be isolated in adapters and quantified in SIL-to-HIL comparison reports.

## 8. Error and status philosophy

- Expected invalid external input returns a typed rejection; it is not an exception.
- Configuration errors are detected before execution and expose a diagnostic string.
- Unknown enum values map to stable `UNKNOWN_*` strings for diagnostics but are rejected before state changes.
- Internal programming errors may use assertions in tests, but production error behavior must remain explicit.
- A failure to generate or validate evidence causes the corresponding CI job to fail.

## 9. Assurance architecture

The build exposes independently selectable controls:

- warnings as errors;
- AddressSanitizer and UndefinedBehaviorSanitizer;
- GCC or Clang coverage instrumentation;
- clang-tidy;
- deterministic unit and scenario tests;
- a controlled mutation that disables CRC rejection;
- provenance manifests containing source hashes and toolchain data.

GitHub Actions separates normal build, full verification, and assurance jobs so a failure is attributable to a specific evidence class.

## 10. Evidence architecture

Generated evidence is not treated as authoritative without provenance. Each full verification run generates or uploads:

- test and scenario reports;
- requirement-check results;
- Monte Carlo results and seed;
- deployment-package report;
- source and documentation hashes;
- exact commit and dirty state;
- compiler, CMake, CTest, Python, host, and architecture information;
- command list and verification disposition.

Large or reproducible build products remain CI artifacts rather than committed binary clutter.

## 11. Planned architectural increments

1. Integrate the scheduler with the application service execution order.
2. Add STANDBY and TEST while preserving existing interface values.
3. Add a time service and ground-command freshness checks.
4. Separate FDIR policy from mode-transition mechanics.
5. Add event and persistent fault records.
6. Add configuration schema validation and versioning.
7. Add fake command and telemetry transports for integration testing without UDP.
8. Add coverage-guided command fuzzing and a frozen corpus.
9. Add per-module coverage and timing reports.
10. Add a read-only, permissioned assurance assistant only after the core evidence baseline is stable.
