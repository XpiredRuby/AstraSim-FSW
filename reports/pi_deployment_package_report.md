# Raspberry Pi Deployment Package Report

Result: PASS

## Package

```text
dist/astrasim-fsw-pi.tar.gz
```

## Included target binaries

- `bin/astra_fsw`
- `bin/astra_fsw_command_telemetry_demo`

## Included ground and verification tools

- `tools/send_command.py`
- `tools/telemetry_receiver.py`
- `tools/run_scenario.py`
- `tools/run_hil_smoke_test.py`
- `tools/check_requirements.py`

## Included controlled interfaces

- `config/protocol_manifest.json`
- `docs/REQUIREMENTS.md`
- `docs/VERIFICATION_MATRIX.csv`

## Package metadata

- Size: `56K`
- SHA256: `31a7a02c294f9dd5fdd19fcbcc9d3605ea7466ad2193aa2a242b835baaf763d9`

## Notes

This report verifies that a Raspberry Pi deployment package can be generated from the repository. It does not establish that the current ASTRA-OS branch has executed on Raspberry Pi hardware. A physical target run must produce a separate, provenance-bound evidence report.
