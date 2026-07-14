# Changelog

All notable ASTRA-OS changes are recorded here. Dates use UTC. Semantic-versioned releases begin with ASTRA-OS v1.0.0.

## [1.0.0] — 2026-07-14

### Added

- configurable `CommandAuthorizer` separated from CRC integrity checking;
- typed `REJECTED_UNAUTHORIZED` telemetry acknowledgement;
- bounded `RecoverySupervisor` with SAFE fallback and `REJECTED_RECOVERY_LIMIT`;
- deterministic timestamp-aware scenario commands;
- configurable Monte Carlo per-trial timeout with clean timeout evidence instead of an uncaught harness traceback;
- end-to-end STANDBY TEST timestamp-guard and recovery-failure scenarios;
- ten-case UDP FDIR campaign for every supported fault disposition;
- per-module coverage CSV JSON and Markdown generation in CI;
- reviewed requirement fingerprints and controlled-interface hashes;
- reverse allocation check from every registered CTest to the verification matrix;
- deterministic governed-assistant read and tool permission policy;
- frozen 129-case governed-assistant evaluation;
- current command-authorization recovery and FDIR documentation.

### Changed

- native CTest suite increased from 18 to 20;
- declared deterministic scenario set increased from five to eight;
- aggregate verification now runs the ten-case FDIR campaign and frozen permission evaluation;
- canonical requirement set contains 67 implemented and reviewed requirements with no planned rows;
- deployment package includes the FDIR campaign and operational-policy documents and no longer ships an unusable source-level traceability checker;
- architecture risk assurance project-state README and verification matrix documents are reconciled with the implemented baseline;
- protocol status map includes unauthorized and recovery-limit command dispositions.

### Verified locally on Raspberry Pi

- 20/20 native CTests;
- 8/8 deterministic YAML scenarios;
- 10/10 FDIR cases;
- 24/24 protocol-conformance checks;
- 25/25 Monte Carlo trials with seed `20260626`;
- 28/28 Python tool tests;
- 129/129 governed-assistant evaluation cases;
- zero requirement failures and zero traceability problems.

### Final managed assurance

- source-clean software-under-test commit `bdd207a396c3054e3eeb74479798110e29b3d1eb`;
- complete workflow `overall_status: passed`;
- source cleanliness full verification sanitizer build sanitizer CTest and controlled mutation all passed;
- refreshed Pi timing resource and thermal evidence is preserved in `reports/pi-hil/`.

### Claim boundary

These results are software-engineering evidence for the documented Raspberry Pi and CI configurations. They do not establish certification flight qualification hard-real-time guarantees production security spacecraft-hardware compatibility or defect absence.

## ASTRA-OS Pi assurance closure

The prior merged closure repaired native Pi build integration cross-machine timestamps scenario build-directory propagation protocol status decoding packaging and evidence preservation. It produced the first complete Pi assurance report with 18 CTest suites five deterministic scenarios seeded Monte Carlo sanitizers controlled mutation timing soak and provenance evidence.

## AstraSim-FSW historical baseline

The predecessor baseline at commit `b63dad495ba921e855e21672831edee444502061` documented nine passing CTest suites deterministic scenarios command and telemetry demonstrations Monte Carlo regression deployment packaging and Raspberry Pi execution evidence. Those records remain preserved as historical evidence and are not treated as current results without provenance.
