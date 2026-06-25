# YAML Scenario Runner

The YAML scenario runner executes command/telemetry verification scenarios.

It starts the C++ command/telemetry demo target, sends commands from a YAML file, listens for telemetry, and checks expected command ACK, mode, and fault fields.

## Run

```bash
bash ci/run_local_tests.sh
python3 tools/run_scenario.py scenarios/basic_command_fault.yaml
```

## Example Scenario

```yaml
name: basic_command_fault

steps:
  - command: SET_MODE
    argument: NOMINAL
    expect:
      ack_seq: 1
      ack_cmd: SET_MODE
      ack_status: 0
      mode: NOMINAL
      fault: NONE
```

## Output

Reports are written to:

```text
reports/scenario_<scenario_name>_output.txt
```

## Verification Value

The scenario runner proves the full loop:

```text
YAML scenario -> UDP command -> C++ FSW app -> telemetry ACK -> expected state check -> report
```
