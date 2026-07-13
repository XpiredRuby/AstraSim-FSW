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
- SHA256: `d00ddff08083217b03982dfccfa3667f47ae6d58c8d794927282e96d39813adb`

## Notes

This report verifies that a Raspberry Pi deployment package can be generated from the repository. It does not establish that the current ASTRA-OS branch has executed on Raspberry Pi hardware. A physical target run must produce a separate, provenance-bound evidence report.
