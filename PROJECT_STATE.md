# ASTRA-OS Project State

## Status

ASTRA-OS is evolving the existing AstraSim-FSW repository without discarding working behavior or history. The current baseline commit is `b63dad495ba921e855e21672831edee444502061` (`Add Pi dashboard snapshot`) on `main`.

## Verified baseline

- Repository: `XpiredRuby/AstraSim-FSW`
- Default branch: `main`
- Visibility: public
- Working branch: `astra-os/baseline-audit`
- Core language/build: C++17 with CMake 3.16 minimum; Python ground tools
- Core modules currently compiled: mode manager, command/telemetry packets, UDP command/telemetry, command processor, flight-software app, health monitor, watchdog
- Main executables: `astra_fsw`, `astra_fsw_telemetry_demo`, `astra_fsw_command_telemetry_demo`
- Existing test registration: 9 CTest suites
- Preserved demonstrations: live telemetry; integrated command/telemetry/fault/recovery; Raspberry Pi HIL evidence
- Current documented command sequence: `SET_MODE NOMINAL`, `INJECT_FAULT CPU_OVERLOAD`, `CLEAR_FAULT`, `SET_MODE NOMINAL`
- Current documented expected behavior: `BOOT -> NOMINAL -> DEGRADED_PAYLOAD`, fault clear, recovery to `NOMINAL`

## Baseline evidence status

The repository records:

- `evidence/ctest_results.txt`: 9/9 local suites passing
- `evidence/command_telemetry_demo.txt`: command, fault, clear, and recovery evidence
- `evidence/pi_ctest_results.txt`: Raspberry Pi aarch64 test evidence
- `evidence/pi_command_telemetry_demo.txt`: Raspberry Pi command/telemetry evidence

These artifacts are preserved as historical baseline evidence. They have not yet been independently regenerated in the current ASTRA-OS work session because the execution environment could access GitHub through the repository connector but could not clone the repository through its local network path.

## Governing scope

The attached `02_ASTRA-OS.md` is authoritative. Supporting software-access and source-list documents are secondary. Claims must remain explicitly non-certification claims. Use `DO-178C-informed` only after traceable mappings exist; do not claim compliance, certification, flight qualification, or hardware validation beyond recorded evidence.

## Initial audit findings

1. The existing system is a useful working baseline and must be preserved.
2. The architecture is currently C++17, while the governing plan requests C++20 where practical; migration must be evidence-driven rather than cosmetic.
3. The repository documents 9 test suites, but no verified per-module structural coverage, mutation score, sanitizer result, static-analysis result, or fuzz result has yet been established in this session.
4. The current mode set in the README is narrower than the governing ASTRA-OS mode set. `STANDBY` and `TEST` are not documented as implemented; exact mode names must be reconciled before changing behavior.
5. Existing command and telemetry CRC validation is valuable, but replay protection, version incompatibility handling, duplicate detection, stale-data handling, and malformed-packet campaigns require audit.
6. Existing Pi evidence is historical and should be retained, but future SIL/HIL comparisons need machine-readable manifests and exact build provenance.
7. Existing documentation uses lowercase filenames (`docs/architecture.md`, `docs/requirements.md`). The required living-file names must be reconciled without creating contradictory duplicates.
8. No open issues or pull requests were found through the connected GitHub search during this audit.

## Phase-mapped backlog

### A0 — Baseline preservation

- [x] Identify repository and baseline commit
- [x] Create ASTRA-OS working branch
- [x] Record known executables, modules, tests, demonstrations, and evidence
- [ ] Regenerate clean build/test baseline and preserve toolchain metadata
- [ ] Regenerate command/telemetry/fault demo from a clean build
- [ ] Add a baseline manifest containing commit, compiler, CMake, OS, architecture, commands, hashes, and results
- [ ] Add a non-destructive baseline tag after regenerated evidence passes

### A1 — CONOPS and requirements

- [ ] Audit and normalize existing requirements
- [ ] Define authoritative mode set and transition semantics
- [ ] Define command authority, packet semantics, telemetry validity, timing budgets, and fault responses
- [ ] Establish stable requirement IDs and verification methods

### A2 — Architecture

- [ ] Reconcile living documentation names
- [ ] Document component boundaries, ownership, lifecycle, clocks, transports, and error philosophy
- [ ] Define test seams and fake transports

### A3–A10 — Core and assurance

- [ ] Add warnings-as-errors option and compiler-warning baseline
- [ ] Add sanitizers and static analysis
- [ ] Add deterministic fake transports and scenario seeds
- [ ] Add malformed-packet, stale-data, replay, simultaneous-fault, watchdog, and recovery tests
- [ ] Add coverage reporting with justified exclusions
- [ ] Add mutation testing or controlled-defect effectiveness checks
- [ ] Add one-command build/test/scenario/evidence workflow

### A11–A14 — Digital thread, governed agent, release

Deferred until the flight-software core, requirements, regression evidence, and assurance pipeline are stable.

## Current test results

- Historical repository record: 9/9 CTest suites passing locally and on Raspberry Pi.
- Current ASTRA-OS session: not independently rerun yet.

## Risks and blockers

- Local clone/build is currently blocked by DNS/network access in the execution sandbox, despite successful GitHub connector access.
- Branch-protection configuration has not yet been verified.
- The repository file tree has been partially reconstructed through code search, not yet enumerated from a recursive tree endpoint.
- Existing generated evidence may lack complete provenance until regenerated.

## Exact continuation point

1. Obtain an executable checkout through an available terminal target or GitHub archive path.
2. Run `bash ci/run_local_tests.sh` from a clean checkout and save the complete output plus toolchain metadata.
3. Run the integrated command/telemetry demo and compare it with the preserved expected sequence.
4. Commit a baseline manifest and concise repository audit.
5. Begin the first implementation change: deterministic malformed-command regression tests and build-hardening options, unless baseline evidence reveals a higher-priority defect.
