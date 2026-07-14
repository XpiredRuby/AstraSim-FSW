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
- `tools/run_fdir_campaign.py`

## Included controlled interfaces

- `config/protocol_manifest.json`
- `docs/REQUIREMENTS.md`
- `docs/VERIFICATION_MATRIX.csv`

## Package metadata

- Size: `64K`
- SHA256: `0811bbad2ef0c559d44823512397ebe3bbf4cfc0f34bdc8f854c192e2631d76f`

## Notes

This report verifies that a Raspberry Pi deployment package can be generated from the repository. It does not establish that the current ASTRA-OS branch has executed on Raspberry Pi hardware. A physical target run must produce a separate, provenance-bound evidence report.
