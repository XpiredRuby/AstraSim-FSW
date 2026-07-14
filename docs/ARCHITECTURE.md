# ASTRA-OS Software Architecture

## 1. Purpose and boundary

ASTRA-OS is a deterministic C++17 spacecraft-style flight-software and assurance platform with Python ground tooling. It demonstrates mode management, UDP command and telemetry protocols, command guards and authorization policy, health and watchdog monitoring, FDIR dispositions, bounded recovery supervision, rate-group execution, configuration control, event evidence, Raspberry Pi execution, and automated verification.

It is educational and portfolio focused. It is not certified, flight qualified, airworthy, hard-real-time, production ready, or a production avionics security design.

## 2. Architectural principles

1. Preserve stable command and telemetry values unless a controlled interface revision authorizes a change.
2. Keep decision logic deterministic and driven by explicit inputs.
3. Reject malformed, replayed, stale, future, or unauthorized external commands before state mutation.
4. Keep internal fault processing distinct from ground-command policy.
5. Validate configuration before execution and expose typed failure dispositions.
6. Generate evidence with exact source, toolchain, workload, and platform provenance.
7. Never treat an AI-generated statement as automatic requirement verification.

## 3. Component and data flow

```text
Python command sender or scenario harness
    |
    v
UDP command receiver
    |
    v
CommandPacket decoder
  - fixed 26-byte format
  - magic and version
  - command ID
  - CRC-16-CCITT
    |
    v
FlightSoftwareApp ground boundary
  - GroundCommandGuard timestamp and sequence policy
  - CommandAuthorizer execution policy
    |
    v
CommandProcessor
  - semantic command validation
  - RecoverySupervisor
  - FDIRManager fault disposition
    |
    v
ModeManager and active fault state

HealthMonitor --------+
Watchdog --------------+--> internal fault selection --> CommandProcessor

FlightSoftwareExecutive
  - RateGroupScheduler
  - flight-service dispatch
  - housekeeping accounting
    |
    v
TelemetryPacket and EventLogger
    |
    v
UDP telemetry sender --> Python receiver and scenario verifier
```

## 4. Core components

### 4.1 `ModeManager`

Owns BOOT, NOMINAL, DEGRADED_SENSOR, DEGRADED_PAYLOAD, SAFE, RECOVERY, STANDBY, and TEST. Existing numeric values remain stable. It accepts only declared transitions and leaves state unchanged after a prohibited transition.

### 4.2 `CommandPacket`

Defines the little-endian command format:

- 32-bit magic;
- 16-bit version;
- 32-bit sequence number;
- 64-bit timestamp in milliseconds;
- 16-bit command ID;
- 32-bit argument;
- 16-bit CRC-16-CCITT.

The decoder performs structural and integrity checks. Semantic command arguments are checked later by `CommandProcessor`.

### 4.3 `GroundCommandGuard`

Applies two independent controls to decoded ground commands:

- bounded age and future-skew checks against the supplied application timestamp;
- unsigned half-range sequence comparison for duplicate, replay, and wrap behavior.

A syntactically valid command that passes the guard consumes its sequence even when semantic or authorization policy later rejects it. This prevents reusing a rejected sequence with altered content.

### 4.4 `CommandAuthorizer`

Applies configurable execution policy after decoding and ground guarding but before state mutation. It can independently disable command classes or entry into TEST. A denial returns `REJECTED_UNAUTHORIZED`, preserves mode and fault state, and is acknowledged in telemetry and events.

This is command policy, not cryptographic sender authentication. CRC is also not authentication.

### 4.5 `CommandProcessor`

Performs semantic validation and deterministic command execution. It owns the active fault state, delegates fault response to `FDIRManager`, and supervises RECOVERY exits through `RecoverySupervisor`.

### 4.6 `RecoverySupervisor`

Starts a bounded session when SAFE enters RECOVERY. Successful exit clears the counter. Repeated prohibited exits increment the counter; reaching the configured limit forces SAFE and returns `REJECTED_RECOVERY_LIMIT`.

### 4.7 `HealthMonitor` and `Watchdog`

Both consume explicit snapshots and return typed reports. Critical health or expired watchdog results become internal fault candidates. They do not pass through the ground replay or authorization policy.

### 4.8 `FDIRManager`

Owns the ten-fault disposition table, severity, priority, response, detection or policy source, and recovery-rule metadata. Simultaneous observations are resolved by highest priority and then lower numeric fault code. A degraded response that is invalid from the current mode falls back to SAFE.

The FDIR policy is verified by unit tests and a ten-case UDP command/telemetry campaign. Some physical detector sources remain outside the software-only implementation and are explicitly identified in `docs/FDIR_MATRIX.md`.

### 4.9 `RateGroupScheduler` and `FlightSoftwareExecutive`

The scheduler is tick driven and has no threads, sleeps, or hidden clock reads. It validates periods, phases, and deadlines; releases jobs deterministically; rejects tick discontinuities; and records completions, overruns, and misses.

`FlightSoftwareExecutive` dispatches the flight-software application and housekeeping service through configured rate groups. Task execution duration is supplied explicitly so scheduler decisions remain independently testable.

### 4.10 `ConfigurationService`

Provides a versioned system schema, value validation, expected-revision comparison, strictly increasing revisions, activation, and irreversible runtime locking. The application configuration includes command guards, authorization policy, recovery supervision, and event-log capacity.

### 4.11 `EventLogger`

Records typed command dispositions, mode transitions, and fault transitions with timestamp, severity, source, code, and message. Capacity is bounded and dropped-event count is observable.

### 4.12 Telemetry

Telemetry includes sequence, timestamp, mode, active fault, resource values, heartbeat, and the last attempted ground-command sequence, ID, and status. Accepted and rejected attempts are visible. Protocol values are mirrored in `config/protocol_manifest.json` and checked against C++ and Python definitions.

## 5. Execution model

The portable core is single-threaded and step driven. All state changes occur in explicit calls. UDP processes and Python tools are external adapters. No hidden worker thread is used in the core.

The Raspberry Pi build uses the same library, tests, scenarios, and protocol manifest as host CI. The timing campaign runs faster than mission time and measures host execution cost; it is not a hard-real-time or worst-case-execution-time proof.

## 6. Clock model

Core services receive integer timestamps or ticks. The C++ core does not read the wall clock. The command/telemetry demo uses Unix wall-clock milliseconds only at its external adapter boundary so commands created on a separate machine use a comparable epoch.

Tests and scenarios may inject exact or relative command timestamps. Timestamp validity does not establish UTC quality or secure time synchronization.

## 7. Interface boundaries

### Core boundary

Typed C++ structures enter and leave the core. Core decision logic does not depend on YAML, files, sockets, or report formatting.

### Transport boundary

UDP adapters own socket lifecycle and packet movement. Packet decoders own byte validation. Transport errors are not silently converted into valid commands.

### Verification boundary

Python tools own scenarios, Monte Carlo generation, FDIR campaigns, protocol comparison, traceability, packaging, and reports. Randomized work publishes seed and configuration.

### Governed-assistant boundary

`tools/assurance_assistant.py` is a deterministic permission interface. It can read only approved project sources and invoke only six exact verification commands. It explicitly denies merge, push, shell, hardware, write, deletion, and automatic verification disposition. The 129-case frozen evaluation applies only to this implemented interface.

## 8. Digital thread

The canonical digital thread consists of:

- `docs/REQUIREMENTS.md`;
- `docs/VERIFICATION_MATRIX.csv`;
- declared YAML scenarios;
- CMake-registered CTests;
- committed evidence reports;
- `config/traceability_baseline.json`.

`tools/check_requirements.py` checks missing and unknown rows, scenario references, registered CTests without requirement allocation, changed requirement fingerprints, and changed controlled-interface hashes. Baseline hashes are updated only after review.

## 9. Assurance architecture

Independent build and workflow controls include:

- warnings as errors;
- normal CTest;
- ASan and UBSan;
- clang-tidy;
- LCOV aggregate and per-module reports;
- controlled CRC mutation;
- bounded libFuzzer execution with deterministic corpus;
- deterministic scenarios and a ten-case FDIR campaign;
- seeded Monte Carlo regression;
- Pi deployment packaging;
- timing and soak campaigns;
- provenance manifests;
- frozen assurance-assistant permission evaluation.

A passing mechanism is evidence only for the exact executed configuration. Coverage percentages, fuzz runs, sanitizers, and timing measurements do not prove absence of defects.

## 10. Remaining external limitations

The completed software baseline does not include or prove:

- cryptographic command authentication or mission key management;
- spacecraft bus drivers or representative avionics hardware;
- actuator command and feedback integration;
- radiation tolerance or single-event-effect recovery;
- mission-derived hard-real-time scheduling guarantees;
- physical detector calibration for every FDIR policy input;
- flight certification or production operations.
