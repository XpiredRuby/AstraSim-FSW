# ASTRA-OS Portfolio Case Study

![ASTRA-OS architecture](assets/astra_os_architecture.svg)

## Project snapshot

ASTRA-OS is a deterministic C++17/Python spacecraft-style flight-software and assurance platform developed from an existing working command/telemetry demonstration. The project preserves the predecessor repository history and externally visible behavior while adding explicit scheduling, stronger command semantics, fault-management policy, recovery supervision, configuration governance, requirements traceability, Raspberry Pi execution, and repeatable assurance evidence.

This is an educational and portfolio project. It is not certified, flight qualified, airworthy, hard-real-time qualified, radiation tolerant, or production spacecraft software.

## Engineering problem

The starting repository demonstrated basic modes, UDP commands, telemetry, health monitoring, watchdog behavior, scenarios, and Raspberry Pi execution. It did not yet provide a coherent assurance baseline that could answer questions such as:

- Which requirements are implemented, and what evidence verifies each one?
- Can malformed, duplicate, replayed, stale, future-dated, or unauthorized commands change state?
- Are fault responses deterministic when multiple faults occur together?
- Is recovery behavior bounded when repeated exit attempts fail?
- Can the same software be rebuilt and exercised reproducibly on Raspberry Pi?
- Are sanitizer, fuzz, mutation, timing, and provenance results tied to an exact source state?

## Delivered architecture

### Deterministic flight-software core

- Eight stable operating modes: `BOOT`, `NOMINAL`, `DEGRADED_SENSOR`, `DEGRADED_PAYLOAD`, `SAFE`, `RECOVERY`, `STANDBY`, and `TEST`.
- A pure tick-driven `RateGroupScheduler` with periods, phases, release records, completion tracking, overruns, and deadline misses.
- A `FlightSoftwareExecutive` that dispatches application and housekeeping work from scheduler releases.
- Health, watchdog, event, configuration, mode, and command-processing services with explicit typed inputs and results.

### Command and telemetry boundary

- Fixed 26-byte command and 43-byte telemetry packet layouts.
- CRC-16-CCITT, magic, version, packet-length, and command-ID validation.
- Wrap-aware duplicate and replay protection using unsigned serial-number arithmetic.
- Bounded timestamp freshness and future-skew checks.
- A separate `CommandAuthorizer` policy layer so execution permission is not conflated with packet integrity.
- Telemetry acknowledgement of accepted and rejected command attempts.

### Fault management and recovery

- Ten explicit FDIR dispositions with severity, priority, response, latching intent, detection source, and recovery rule.
- Order-independent simultaneous-fault selection with deterministic tie-breaking.
- SAFE fallback when a degraded response is not valid from the current mode.
- A configurable `RecoverySupervisor` that forces SAFE after repeated prohibited recovery-exit attempts.

### Assurance and digital thread

- Canonical requirements and verification matrix with stable IDs.
- Requirement fingerprints and controlled-interface hashes.
- Reverse checks that every registered CTest is allocated to at least one requirement.
- One-command verification covering native tests, scenarios, fault campaigns, protocol consistency, randomized regression, packaging, traceability, and permission evaluation.
- ASan/UBSan, clang-tidy, structural coverage, bounded libFuzzer, and controlled mutation workflows.
- Provenance manifests recording commit, source cleanliness, host, toolchain, commands, and input hashes.

## Quantitative evidence

| Verification area | Result |
|---|---:|
| Native CTest suites | **20/20 passed** |
| ASan/UBSan CTest suites | **20/20 passed** |
| Python tooling tests | **27/27 passed** |
| Deterministic YAML scenarios | **8/8 passed** |
| Explicit FDIR cases | **10/10 passed** |
| Seeded Monte Carlo trials | **25/25 passed** |
| C++/Python protocol checks | **24/24 passed** |
| Governed-permission evaluation | **129/129 passed** |
| Requirement failures | **0** |
| Traceability problems | **0** |
| Planned canonical requirements | **0** |
| Controlled mutation | **Killed / PASS** |
| Managed assurance workflow | **PASS** |

The definitive assurance campaign was run against source commit `bdd207a396c3054e3eeb74479798110e29b3d1eb`. The completed project was merged into `main` at `0707a0e82fa208a786015813ecec704b996686e0` before release preparation.

## Raspberry Pi evidence

The same portable core was built and executed natively on Ubuntu 24.04 `aarch64` with a Raspberry Pi kernel. The completion campaign included:

- 250,000 nominal ticks with zero deadline misses;
- 100,000 controlled-overrun ticks with 99 injected misses detected;
- 1,000,000 soak ticks with zero deadline misses;
- process wall time, CPU time, RSS, page-fault, context-switch, and temperature evidence;
- reproducible target packaging with checksum.

These measurements demonstrate observed faster-than-real-time host execution on the documented Pi. They are not worst-case execution-time or hard-real-time qualification evidence.

## Most important engineering decisions

1. Preserve the working repository and history instead of rebuilding from scratch.
2. Keep packet decoders pure and enforce replay/freshness at the application boundary.
3. Separate command authorization from CRC integrity and transport decoding.
4. Keep the scheduler callback-free so release decisions remain independently testable.
5. Treat evidence as generated, provenance-bound output rather than as an unsupported claim.
6. Prevent automated tooling from marking requirements verified or performing unrestricted repository/hardware actions.

## Recruiter-ready description

> Developed ASTRA-OS, a C++17/Python spacecraft-style flight-software and assurance platform with deterministic scheduling, UDP command/telemetry protocols, replay and freshness protection, command authorization policy, ten-case FDIR, bounded recovery supervision, Raspberry Pi execution, requirements traceability, ASan/UBSan, fuzzing, mutation testing, Monte Carlo regression, and provenance-bound evidence. Delivered 20/20 native and sanitizer CTests, 8/8 deterministic scenarios, 10/10 fault cases, 25/25 randomized trials, and zero traceability failures.

## Primary evidence

- `reports/ASTRA_OS_FINAL_COMPLETION_REPORT.md`
- `reports/latest/assurance_summary.json`
- `reports/latest/baseline_manifest.json`
- `reports/fdir_campaign_report.md`
- `reports/assurance_assistant_eval.md`
- `reports/requirement_check_report.md`
- `reports/pi-hil/`
