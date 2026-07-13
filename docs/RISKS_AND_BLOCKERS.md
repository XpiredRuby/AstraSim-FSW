# ASTRA-OS Risks, Limitations, and Blockers

## Status convention

- **Closed**: the identified software defect or evidence gap has been repaired and verified.
- **Residual limitation**: intentionally outside the current educational software claim.
- **Operational dependency**: requires infrastructure or owner action but does not block local software verification.

## Closed software and assurance risks

| Item | Resolution | Evidence |
|---|---|---|
| Missing native Pi build sources and tests | CMake now includes command guard configuration service command authorizer recovery supervisor and all registered tests | `CMakeLists.txt`; native CTest evidence |
| Cross-machine timestamp epochs were incomparable | External command and demo adapters use Unix milliseconds while the core still receives explicit time | `tools/send_command.py`; `fsw/src/command_telemetry_demo.cpp`; timestamp scenario |
| Duplicate replay stale and future commands could lack end-to-end evidence | Ground guard and timestamp-aware scenario harness verify typed rejections | `ground_command_guard_tests`; `scenarios/command_timestamp_guard.yaml` |
| STANDBY and TEST were documented as planned after implementation | Canonical requirements architecture and scenario evidence are reconciled | `extended_mode_tests`; `scenarios/extended_modes.yaml` |
| Command execution policy was not separated from CRC integrity | `CommandAuthorizer` now applies configurable policy before state mutation | `command_authorizer_tests`; `docs/command_authorization.md` |
| RECOVERY could be retried indefinitely | Bounded `RecoverySupervisor` forces SAFE after the configured failure limit | `recovery_supervisor_tests`; recovery fail-safe scenario |
| FDIR enum count was used without ten independent cases | Ten supported faults now execute through a deterministic UDP campaign | `reports/fdir_campaign_report.md` |
| Per-module coverage reporting was not produced by CI | Coverage workflow now generates CSV JSON and Markdown module reports | `.github/workflows/assurance.yml`; `tools/summarize_lcov.py` |
| Registered tests could exist without requirement allocation | Enhanced checker reverse-maps every CTest to the verification matrix | `tools/check_requirements.py`; `tools/tests/test_traceability.py` |
| Requirement or controlled-interface changes could bypass review | Reviewed fingerprints and interface hashes are frozen in configuration | `config/traceability_baseline.json` |
| Assurance-assistant permissions were qualitative only | Deterministic policy and 129-case frozen evaluation now quantify allow and deny behavior | `reports/assurance_assistant_eval.md` |

## Residual limitations

### Cryptographic command security

CRC and the command-authorization policy do not authenticate a sender. Production secure commanding would require mission-approved cryptography key management identities roles secure updates and incident procedures.

### Hardware and bus integration

The project does not include spacecraft bus drivers actuator interfaces power switching radiation testing or representative flight hardware. Raspberry Pi execution validates the portable software on ARM64 Linux only.

### Real-time guarantees

Timing campaigns measure faster-than-real-time host execution under documented conditions. They do not provide worst-case execution-time proof priority-inversion analysis schedulability certification or hard-real-time guarantees.

### Physical FDIR detection

All ten policy dispositions are independently injected and verified. Several physical detector sources remain policy inputs rather than hardware-integrated sensors. See `docs/FDIR_MATRIX.md`.

### Coverage and fuzzing interpretation

Coverage and bounded fuzzing improve evidence but cannot prove defect absence. Numerical coverage claims must identify module exclusions toolchain exact commit and workload.

### Certification

No evidence in this repository establishes DO-178C compliance flight qualification airworthiness production readiness or spacecraft compatibility.

## Operational dependencies

| Dependency | Effect | Mitigation |
|---|---|---|
| GitHub Actions availability | Remote CI artifacts and status checks may be delayed | Native Pi and local assurance workflows remain reproducible |
| Raspberry Pi tunnel availability | Remote connector operations pause when the tunnel drops | Managed processes and local evidence prevent repeating completed stages |
| Exact Pi board model not exposed by restricted connector | Board and SoC cannot be claimed precisely | Reports identify only verified Ubuntu kernel and `aarch64` facts |
| Repository owner review for protected merges | Branch may require a pull-request approval path | Push a verified branch and preserve a complete PR description |

## No current software blocker

There is no known blocker to building testing packaging or reviewing the current ASTRA-OS software baseline. Remaining items are claim boundaries or external mission/hardware decisions rather than unfinished portable-core defects.
