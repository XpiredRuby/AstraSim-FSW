# Recommended Pull Request

## Title

Repair native Raspberry Pi integration and close ASTRA-OS assurance evidence

## Branches

- Base: `astra-os/baseline-audit`
- Compare: `astra-os/pi-hil-build-fix`

This should be a separate stacked repair PR because the fix branch is based directly on `origin/astra-os/baseline-audit`. Targeting the baseline branch keeps the review focused on the five Pi repair/closure commits rather than repeating the full baseline history. Do not merge without explicit approval.

## Pull Request Body

### Summary

This PR repairs integration defects discovered during native Raspberry Pi execution and completes the ASTRA-OS Pi verification, assurance, packaging, traceability, documentation, and evidence-closure workflow.

The software-under-test for the final Pi assurance campaign was commit `395f1e028a3ab6bf46438b4dbd5bad256eced0cd`. Documentation and preserved evidence were committed afterward.

### Native Pi defects repaired

- add missing `ground_command_guard.cpp` and `configuration_service.cpp` core sources to CMake;
- add `astra_timing_campaign`;
- add and register ground-command guard and configuration-service tests;
- restore required `ConfigurationService` private state and include support;
- pass `FlightSoftwareAppConfig` through `FlightSoftwareExecutiveConfig`;
- combine executive scheduler/app validity and validation errors;
- replace incomparable cross-machine monotonic timestamps with Unix/system-clock milliseconds;
- align command timestamps in simulated app tests;
- add `--build-dir` support to scenario, HIL, Monte Carlo, package, aggregate-verification, and assurance workflows;
- normalize numeric acknowledgement expectations through the authoritative status map;
- add stale, future, and guard-configuration command-status decoding.

### Final verification results

- native Raspberry Pi Release build: PASS;
- CTest: 18/18 passed;
- deterministic scenarios: 5/5 passed;
- protocol conformance: 24/24 passed;
- requirement traceability: `FAIL=0`, `TRACEABILITY_PROBLEMS=0`;
- seeded Monte Carlo: 25/25 passed with seed `20260626`;
- ASan/UBSan warnings-as-errors build and tests: PASS;
- controlled mutation: PASS;
- deployment package generation from `build-pi`: PASS;
- nominal timing campaign: 250,000 ticks, 0 deadline misses;
- controlled-overrun campaign: 100,000 ticks, 99 injected misses detected;
- soak campaign: 1,000,000 ticks, 0 deadline misses;
- final managed assurance workflow: `overall_status: passed`.

### Evidence

- `reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md`
- `reports/latest/assurance_summary.json`
- `reports/latest/baseline_manifest.json`
- `reports/latest/protocol_conformance.json`
- `reports/latest/full_verification.log`
- `reports/latest/test_sanitizers.log`
- `reports/latest/controlled_mutation.log`
- `reports/pi-hil/`
- `reports/monte_carlo_report.md`
- `reports/requirement_check_report.md`
- `reports/pi_deployment_package_report.md`
- `reports/scenario_*_output.txt`

### Deployment package

- archive: `dist/astrasim-fsw-pi.tar.gz` (generated locally and intentionally ignored by Git);
- SHA256: `d00ddff08083217b03982dfccfa3667f47ae6d58c8d794927282e96d39813adb`.

### Claim boundary

This is educational and portfolio-focused evidence. The results do not establish certification, flight qualification, hard-real-time guarantees, airworthiness, production readiness, radiation tolerance, or spacecraft-hardware compatibility.

The Pi timing campaigns are host faster-than-real-time execution measurements, not real-time qualification or worst-case execution-time proof.

### Review notes

- The old preservation worktree was not modified.
- No force push was used.
- Generated evidence is tied to the tested commit and platform provenance recorded in the baseline manifest.
- The exact Raspberry Pi board/SoC model was not exposed by the restricted connector and was not guessed.
- Merge requires explicit owner approval.
