# HIL Smoke Test

The HIL smoke test verifies the same command and telemetry interfaces used by a hardware-in-the-loop setup.

In the current local configuration, the flight software demo process acts as the embedded target and the Python tooling acts as the external test controller. On a Raspberry Pi target, the same command and telemetry protocol can be exercised across the network.

## Command

```bash
python3 tools/run_hil_smoke_test.py
```

## Scenario

```text
scenarios/hil_smoke_test.yaml
```

## Evidence

The test writes:

```text
reports/scenario_hil_smoke_test_output.txt
```

## Verified behavior

The HIL smoke test verifies:

- external command injection over UDP
- telemetry packet reception over UDP
- command acknowledgement telemetry
- valid BOOT to NOMINAL transition
- CPU overload fault handling
- fault clearing
