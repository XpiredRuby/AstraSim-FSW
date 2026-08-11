# Security Policy

ASTRA-OS is an educational portfolio project, not operational spacecraft software.

## Reporting a vulnerability

Please report security-relevant defects privately to **vinhoustontexas@gmail.com** before opening a public issue when the defect could enable unintended command execution, bypass a documented guard, corrupt the command/telemetry boundary, or materially invalidate the stated assurance evidence.

Include, where possible:

- affected commit or release;
- reproduction steps;
- expected vs. observed behavior;
- whether the issue affects the C++ core, Python tooling, browser model, or CI/evidence pipeline.

## Scope

Security-relevant project areas include:

- packet parsing and CRC handling;
- replay/freshness guards;
- command authorization policy;
- configuration update/lock behavior;
- fault/recovery state transitions;
- verification/evidence integrity.

The project does **not** implement cryptographic sender authentication, key management, classified interfaces, operational spacecraft networking, or a production security boundary. Findings should therefore be framed against the documented software model rather than against capabilities the project does not claim.

## Supported versions

The current `main` branch and the latest tagged release are the maintained public baselines. Historical commits and preserved evidence remain available for reproducibility but are not separately patched.
