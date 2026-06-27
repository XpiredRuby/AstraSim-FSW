# Raspberry Pi Deployment Package Report

Result: PASS

## Package

```text
dist/astrasim-fsw-pi.tar.gz
```

## Included target binaries

- `bin/astra_fsw`
- `bin/astra_fsw_command_telemetry_demo`

## Included verification tools

- `tools/send_command.py`
- `tools/telemetry_receiver.py`
- `tools/run_scenario.py`
- `tools/run_hil_smoke_test.py`
- `tools/check_requirements.py`

## Package metadata

- Size: `36K`
- SHA256: `cdd44d6181212e28d0fd64b55a81bc7e2570070f2d9f426fc11e5427160c2140`

## Notes

This report verifies that a Raspberry Pi deployment package can be generated from the repository. Running the package on physical Raspberry Pi hardware should produce a separate target-run evidence report.
