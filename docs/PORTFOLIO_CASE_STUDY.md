# ASTRA-OS Portfolio Case Study

![ASTRA-OS architecture](assets/astra_os_architecture.svg)

## Project snapshot

ASTRA-OS is a deterministic C++17/Python spacecraft-style flight-software and assurance platform. It evolved from an earlier command/telemetry demonstration into a configuration-controlled software baseline with explicit scheduling, stronger command semantics, fault-management policy, bounded recovery, requirements traceability, native Raspberry Pi execution evidence, and repeatable assurance artifacts.

This is an educational portfolio project. It is not certified, flight-qualified, airworthy, hard-real-time qualified, radiation tolerant, or production spacecraft software.

## Engineering problem

The useful question was not simply whether a state machine could change modes. It was whether a reviewer could answer, from repository evidence:

- Which requirements exist, and what verifies each one?
- Can malformed, duplicate, replayed, stale, future-dated, or unauthorized commands change state?
- Are fault responses deterministic when multiple faults occur together?
- Is recovery bounded when repeated exit attempts fail?
- Can the same portable core be rebuilt and executed on the target architecture?
- Are sanitizer, fuzz, mutation, timing, and provenance results tied to a known source state?

That assurance problem became the main project.

## Delivered architecture

### Deterministic flight-software core

- Eight stable operating modes: `BOOT`, `NOMINAL`, `DEGRADED_SENSOR`, `DEGRADED_PAYLOAD`, `SAFE`, `RECOVERY`, `STANDBY`, and `TEST`.
- A tick-driven `RateGroupScheduler` with periods, phases, release records, completion tracking, overruns, and deadline-miss accounting.
- A `FlightSoftwareExecutive` that dispatches application and housekeeping work from scheduler releases.
- Health, watchdog, event, configuration, mode, and command-processing services with typed inputs and results.

### Command and telemetry boundary

- Fixed 26-byte command and 43-byte telemetry packet layouts.
- CRC-16-CCITT, magic, version, packet-length, and command-ID validation.
- Wrap-aware duplicate and replay protection.
- Bounded timestamp freshness and future-skew checks.
- A separate `CommandAuthorizer` layer so execution permission is not conflated with packet integrity.
- Telemetry acknowledgement of accepted and rejected command attempts.

CRC detects corruption under the tested model. It does not authenticate the sender.

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
- Provenance manifests recording source state, cleanliness, host, toolchain, commands, and input hashes.

## Quantitative evidence

### Current repository tree

| Verification area | Result |
|---|---:|
| Native CTest suites | **20/20 passed** |
| Python tool + browser-model tests | **35/35 passed** |
| Deterministic YAML scenarios | **8/8 passed** |
| Explicit FDIR cases | **10/10 passed** |
| Seeded Monte Carlo trials | **25/25 passed** |
| C++/Python protocol checks | **24/24 passed** |
| Governed-permission evaluation | **129/129 passed** |
| Requirement failures | **0** |
| Traceability problems | **0** |
| Planned canonical requirements | **0** |
| Controlled mutation | **Killed / PASS** |

The current-tree Python count is verified by CI. Frozen release artifacts intentionally retain the smaller counts that existed at their tested commits: the final-completion report records 27 tooling tests and the v1.0.0 release notes record 28. Later browser-model/tool tests raised the current count to 35.

### Frozen completion baseline

The definitive completion campaign was run against source commit `bdd207a396c3054e3eeb74479798110e29b3d1eb`; the exact toolchain, commands, hashes, and evidence paths are preserved in the final completion report and provenance manifest. Historical reports are not rewritten to look current.

## Raspberry Pi evidence

The portable core was built and executed natively on Ubuntu 24.04 `aarch64` with a Raspberry Pi kernel. The collection interface did not expose an authoritative board/SoC model, so the project does not guess one.

The preserved target campaign includes:

- native CTest execution;
- deterministic command/fault scenarios;
- protocol and randomized regression checks;
- 250,000 nominal timing ticks with zero detected deadline misses;
- a controlled-overrun campaign that detected the injected misses;
- a 1,000,000-tick soak campaign;
- process resource observations;
- a reproducible deployment package with checksum.

These are native target-execution and faster-than-real-time process measurements. They are **not** spacecraft hardware-in-the-loop, WCET, schedulability, or hard-real-time qualification evidence.

## Most important engineering decisions

1. Preserve the working repository and its history instead of replacing it with a polished rewrite.
2. Keep packet decoding separate from replay/freshness policy.
3. Separate command authorization from CRC integrity and transport decoding.
4. Keep scheduler release decisions independently testable.
5. Treat evidence as provenance-bound output rather than as an unsupported claim.
6. Demonstrate that at least one critical test family can kill a controlled defect rather than assuming a green test suite is meaningful.
7. Prevent automated tooling from marking requirements verified or performing unrestricted repository/hardware actions.

## What failed or changed during validation

Native target work exposed integration defects that host-side work had not caught, including missing CMake sources/tests, incomplete configuration plumbing, cross-machine timestamp assumptions, incomplete status mappings, and build-directory assumptions in scenario/deployment tooling. The fixes and original failure modes are retained in the Raspberry Pi verification report rather than edited out of the history.

That is one of the strongest results of the project: target execution changed the software.

## Recruiter-ready description

> Developed ASTRA-OS, a C++17/Python spacecraft-style flight-software and assurance platform with deterministic scheduling, UDP command/telemetry, replay and freshness protection, execution authorization policy, ten-case FDIR, bounded recovery, configuration control, native Raspberry Pi/aarch64 execution evidence, requirements traceability, sanitizers, fuzzing, controlled mutation, Monte Carlo regression, and provenance-bound verification. Current CI exercises 20/20 CTest suites, 35/35 Python/tool tests, 8/8 deterministic scenarios, 10/10 fault cases, 24/24 protocol checks, 25/25 seeded trials, and 129/129 frozen permission cases.

## Primary evidence

- `reports/ASTRA_OS_FINAL_COMPLETION_REPORT.md`
- `reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md`
- `reports/latest/assurance_summary.json`
- `reports/latest/baseline_manifest.json`
- `reports/fdir_campaign_report.md`
- `reports/assurance_assistant_eval.md`
- `reports/requirement_check_report.md`
- `reports/pi-hil/` (historical directory name)
