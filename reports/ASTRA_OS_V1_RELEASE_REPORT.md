# ASTRA-OS v1.0.0 Release Verification Report

## Release disposition

**Result: PASS**

ASTRA-OS v1.0.0 is the first semantic-versioned portfolio release of the completed portable flight-software and assurance baseline.

The exact source-under-test for the definitive release campaign was:

```text
759d2a1c3e617a0aca7e4414baee9710e0f145ca
```

The provenance manifest records `source_dirty: false`, and the managed assurance summary records `overall_status: passed`.

## Release-facing additions

- CMake project version promoted to `1.0.0`.
- Dated v1.0.0 changelog entry.
- Release notes suitable for the GitHub tag annotation and release page.
- Rendered SVG architecture overview integrated into the README.
- Recruiter-readable engineering case study.
- Ready-to-record 90-second demonstration script and shot list.
- Hardened Monte Carlo subprocess timeout handling with a configurable 45-second default and regression test.

## Definitive verification results

| Verification area | Result |
|---|---:|
| Native CTest suites | 20/20 passed |
| ASan/UBSan CTest suites | 20/20 passed |
| Python tooling tests | 28/28 passed |
| Deterministic scenarios | 8/8 passed |
| FDIR command/telemetry cases | 10/10 passed |
| Seeded Monte Carlo | 25/25 passed; seed `20260626` |
| Protocol conformance | 24/24 passed |
| Governed-assurance evaluation | 129/129 passed |
| Requirement failures | 0 |
| Traceability problems | 0 |
| Planned canonical requirements | 0 |
| Controlled CRC mutation | PASS |
| Deployment package generation | PASS |
| Managed assurance workflow | PASS |

## Verification stages

The final managed workflow executed and passed:

1. source-cleanliness classification;
2. all Python tooling tests;
3. complete build, native CTest, deterministic scenarios, FDIR campaign, Monte Carlo, package generation, protocol checking, requirements checking, and permission evaluation;
4. warnings-as-errors sanitizer configuration and build;
5. all CTests under AddressSanitizer and UndefinedBehaviorSanitizer;
6. controlled CRC mutation effectiveness check;
7. provenance-manifest generation.

## Monte Carlo timeout defect found during release closure

The first release-candidate run exposed a verification-harness weakness: a generated scenario exceeded a hard-coded 20-second subprocess timeout under Raspberry Pi load, and the uncaught `TimeoutExpired` exception aborted the campaign.

The correction:

- raises the default per-trial limit to 45 seconds;
- exposes `--trial-timeout-s`;
- records a timeout as an explicit failed trial with partial diagnostics;
- prevents an infrastructure timeout from terminating the campaign with an uncaught traceback;
- adds a regression test for the timeout path.

After the correction, the deterministic seeded campaign passed 25/25 and the full managed assurance workflow passed.

## Primary evidence

- `RELEASE_NOTES_v1.0.0.md`
- `docs/assets/astra_os_architecture.svg`
- `docs/PORTFOLIO_CASE_STUDY.md`
- `docs/PORTFOLIO_DEMO.md`
- `reports/latest/assurance_summary.json`
- `reports/latest/baseline_manifest.json`
- `reports/latest/v1_release_assurance_final.log`
- `reports/latest/v1_release_assurance_timeout_failure.log`
- `reports/latest/python_tool_tests.log`
- `reports/latest/full_verification.log`
- `reports/latest/test_sanitizers.log`
- `reports/latest/controlled_mutation.log`
- `reports/monte_carlo_report.md`
- `reports/fdir_campaign_report.md`
- `reports/requirement_check_report.md`
- `reports/assurance_assistant_eval.md`

## Claim boundary

This release is educational and portfolio-focused. It does not establish certification, DO-178C compliance, flight qualification, airworthiness, production readiness, cryptographic sender authentication, radiation tolerance, operational spacecraft compatibility, worst-case execution-time bounds, hard-real-time guarantees, or defect absence.
