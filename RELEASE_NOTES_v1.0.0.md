# ASTRA-OS v1.0.0 Release Notes

## Release scope

ASTRA-OS v1.0.0 is the first complete software and assurance baseline of the AstraSim-FSW evolution project. It preserves the predecessor command/telemetry demonstrations and repository history while delivering deterministic scheduling, stronger command semantics, explicit FDIR policy, bounded recovery, configuration governance, Raspberry Pi execution, requirements traceability, and provenance-bound verification evidence.

## Highlights

- Eight stable operating modes with preserved on-wire values.
- Deterministic rate-group scheduling and executive dispatch.
- Fixed binary UDP command and telemetry protocols with CRC validation.
- Duplicate, replay, stale-timestamp, and future-timestamp rejection.
- Command authorization policy separated from integrity validation.
- Ten explicit FDIR dispositions with deterministic simultaneous-fault priority.
- Bounded recovery-failure supervision that forces SAFE.
- Event logging and revisioned, lockable configuration service.
- Native Raspberry Pi build, timing, soak, resource, and packaging evidence.
- Requirements, verification matrix, interface fingerprints, and reverse test allocation.
- ASan/UBSan, clang-tidy, structural coverage, bounded libFuzzer, controlled mutation, and seeded Monte Carlo workflows.
- Deterministic governed-assurance boundary with 129 frozen permission cases.

## Definitive verification results

| Verification area | Result |
|---|---:|
| Native CTest suites | 20/20 passed |
| ASan/UBSan CTest suites | 20/20 passed |
| Python tooling tests | 27/27 passed |
| Deterministic scenarios | 8/8 passed |
| FDIR cases | 10/10 passed |
| Seeded Monte Carlo | 25/25 passed |
| Protocol conformance | 24/24 passed |
| Permission evaluation | 129/129 passed |
| Requirement failures | 0 |
| Traceability problems | 0 |
| Planned canonical requirements | 0 |
| Controlled mutation | PASS |
| Managed assurance | PASS |

## Provenance

The annotated `v1.0.0` tag identifies the published release commit. The exact software-under-test commit, repository cleanliness, toolchain, commands, and source hashes are recorded in `reports/latest/baseline_manifest.json`.

- Final report: `reports/ASTRA_OS_FINAL_COMPLETION_REPORT.md`
- Machine summary: `reports/latest/assurance_summary.json`
- Provenance manifest: `reports/latest/baseline_manifest.json`

## Portfolio assets

- Architecture visual: `docs/assets/astra_os_architecture.svg`
- Recruiter-readable case study: `docs/PORTFOLIO_CASE_STUDY.md`
- Ready-to-record demo script: `docs/PORTFOLIO_DEMO.md`

## Claim boundary

ASTRA-OS is an educational and portfolio software-engineering project. This release does not establish certification, DO-178C compliance, flight qualification, airworthiness, production readiness, radiation tolerance, compatibility with operational spacecraft hardware, worst-case execution-time bounds, or hard-real-time guarantees.
