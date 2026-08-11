# ASTRA-OS Software Assurance Workflow

## Claim boundary

This workflow produces repeatable software-engineering evidence for an educational and portfolio project. It does not establish certification, DO-178C compliance, flight qualification, hard-real-time guarantees, airworthiness, production readiness, cybersecurity certification, or operational reliability.

## One-command workflow

Run the complete workflow against an explicit native build directory:

```bash
python3 tools/run_astra_os_assurance.py --build-dir build
```

The default workflow performs:

1. a source-cleanliness gate that permits generated evidence changes but rejects uncommitted source, configuration, workflow, or documentation changes;
2. all Python tooling tests plus the native build and all registered CTests;
3. all declared deterministic YAML scenarios;
4. the ten-case FDIR command/telemetry campaign;
5. seeded Monte Carlo regression;
6. Raspberry Pi deployment-package generation;
7. C++/Python/manifest protocol conformance;
8. requirement-matrix reverse-test and reviewed-hash traceability checks;
9. the frozen assurance-assistant permission evaluation;
10. warnings-as-errors ASan and UBSan build and tests;
11. a controlled CRC mutation that the command tests must kill;
12. machine-readable assurance and source/toolchain provenance output.

For a faster regression that skips sanitizer and mutation stages:

```bash
python3 tools/run_astra_os_assurance.py --build-dir build --quick
```

Generated transient evidence is written under `reports/latest/`. Selected reviewed release evidence may be committed intentionally.

## Deterministic verification layers

### CTest

The native C++ suite covers mode management, command packets, UDP adapters, command processing, command authorization, recovery supervision, the application, health monitoring, watchdog behavior, scheduling, extended modes, FDIR, simultaneous faults, executive dispatch, event logging, command guarding, and configuration-service behavior.

### Declared YAML scenarios

The repository declares eight end-to-end command/telemetry scenarios:

- basic command and CPU-fault flow;
- stale and future timestamp rejection;
- STANDBY and TEST transitions;
- an integration smoke flow;
- invalid transition rejection;
- bounded recovery-failure fallback;
- sensor-timeout SAFE transition;
- watchdog-timeout SAFE transition.

The historical scenario filename `hil_smoke_test.yaml` is retained for provenance. It is a software/integration scenario and should not be interpreted as proof of representative spacecraft hardware-in-the-loop testing.

### Ten-case FDIR campaign

`tools/run_fdir_campaign.py` injects each supported fault through the UDP command/telemetry path and verifies acknowledgement, fault indication, and resulting mode. This verifies software-policy response paths rather than every physical detector.

### Seeded Monte Carlo

`tools/run_monte_carlo.py` uses explicit seed `20260626` by default in the aggregate workflow. Failed trials remain visible and are not discarded.

## Digital-thread integrity

`tools/check_requirements.py` checks:

- missing or unknown requirement and matrix entries;
- status mismatches;
- unknown scenario requirement references;
- missing scenario evidence;
- every CMake-registered CTest has at least one requirement allocation;
- changed requirement fingerprints awaiting review;
- changed controlled-interface hashes awaiting review.

The reviewed hashes are stored in:

```text
config/traceability_baseline.json
```

They are regenerated only after an intentional review:

```bash
python3 tools/update_traceability_baseline.py
```

Updating the baseline is not itself proof that a change is correct. The diff must be reviewed with the requirement and interface changes.

## Governed assurance assistant

`tools/assurance_assistant.py` implements a deterministic permission boundary. It may:

- read files beneath approved project roots;
- invoke six exact verification commands defined in policy.

It explicitly denies merge, push, force-push, arbitrary shell, write, deletion, hardware-command, repository-visibility, and automatic requirement-verification actions.

Policy and frozen evaluation:

```text
config/assurance_assistant_policy.json
config/assurance_assistant_eval.json
```

Run the evaluation with:

```bash
python3 tools/run_assurance_assistant_eval.py
```

The evaluation contains 129 deterministic allow and deny cases. Passing it does not establish general AI safety or security outside this interface.

## Sanitizers and static analysis

The assurance workflow and GitHub Actions provide independently attributable jobs for:

- AddressSanitizer and UndefinedBehaviorSanitizer;
- clang-tidy with findings treated as errors;
- Python compilation and unit tests;
- browser JavaScript syntax;
- ShellCheck;
- protocol-manifest conformance.

## Controlled mutation

`tools/run_controlled_mutation.py` creates an isolated source copy and disables CRC rejection. It builds and runs `command_packet_tests`.

- a detected test failure kills the mutation and passes the experiment;
- test success means the defect survived and fails the experiment;
- a mutation that does not compile is an invalid experiment rather than a killed defect.

## Coverage

The CI coverage job removes system headers and test sources from the production trace and then generates:

- aggregate LCOV summary;
- per-module CSV;
- per-module JSON;
- per-module Markdown with exclusions.

The report is produced by `tools/summarize_lcov.py`. Coverage percentage is not an adequacy or defect-absence claim.

## Fuzzing

The command fuzzer uses `LLVMFuzzerTestOneInput` and checks invariants including output preservation after rejection, accepted-packet round trips, field preservation, and safe enum conversion.

The deterministic seed corpus is generated by:

```bash
python3 tools/generate_command_fuzz_corpus.py --output build-fuzz/command-corpus
```

CI runs a fixed bounded campaign and preserves corpus manifest output and artifacts. A passing bounded run does not prove parser correctness for all inputs.

## Timing and soak evidence

`astra_timing_campaign` produces faster-than-real-time host measurements with explicit tick count, mission step, overload pattern, percentiles, final mode, final fault, and deadline-miss count. `/usr/bin/time` evidence adds CPU and maximum RSS. Preserved Pi evidence also records observed temperature ranges.

These are execution-cost observations, not hard-real-time WCET or schedulability proofs.

## Deployment package

`tools/package_pi_deployment.sh --build-dir build` packages the portable binaries, scenarios, ground tools, protocol manifest, and operational-policy documentation. Package generation is a separate claim from native target execution.

## Provenance

`tools/generate_baseline_manifest.py` records:

- commit, branch, remote, describe, full dirty state, and source-versus-generated-evidence dirty paths;
- OS, architecture, and Python;
- compiler, CMake, CTest, and Git probes;
- build and instrumentation options;
- executed verification commands;
- GitHub metadata when available;
- SHA-256 hashes and sizes for source, tests, scenarios, workflows, tools, configuration, and documentation.

## Raspberry Pi evidence

Preserved native Raspberry Pi/aarch64 execution evidence lives primarily in `reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md` and the historical `reports/pi-hil/` directory. The directory name is historical; the demonstrated claim is native target execution on the recorded Linux/aarch64 environment, not representative spacecraft HIL.

A complete assurance run is accepted only when the process exits successfully and `reports/latest/assurance_summary.json` records `overall_status: passed`.

## Primary evidence

- `reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md`
- `reports/requirement_check_report.md`
- `reports/fdir_campaign_report.md`
- `reports/assurance_assistant_eval.md`
- `reports/monte_carlo_report.md`
- `reports/pi_deployment_package_report.md`
- `reports/pi-hil/` (historical directory name)
- selected `reports/latest/` release artifacts
