# Raspberry Pi Deployment

AstraSim-FSW can be packaged for deployment to a Raspberry Pi target. The package contains the flight software binaries, command/telemetry tools, scenarios, and documentation needed to run a target smoke test.

## Build package

From the repository root:

```bash
bash tools/package_pi_deployment.sh
```

The package is written to:

```text
dist/astrasim-fsw-pi.tar.gz
```

A package evidence report is written to:

```text
reports/pi_deployment_package_report.md
```

## Copy package to Raspberry Pi

Example:

```bash
scp dist/astrasim-fsw-pi.tar.gz pi@raspberrypi.local:~
```

On the Pi:

```bash
tar -xzf astrasim-fsw-pi.tar.gz
cd astrasim-fsw-pi
```

## Run target command/telemetry smoke test

Terminal 1:

```bash
./bin/astra_fsw_command_telemetry_demo 6000 127.0.0.1 5005 20
```

Terminal 2:

```bash
python3 tools/telemetry_receiver.py --port 5005
```

Terminal 3:

```bash
python3 tools/send_command.py --port 6000 --sequence 1 --command SET_MODE --argument NOMINAL
python3 tools/send_command.py --port 6000 --sequence 2 --command INJECT_FAULT --argument CPU_OVERLOAD
python3 tools/send_command.py --port 6000 --sequence 3 --command CLEAR_FAULT
```

Expected behavior:

- command acknowledgements appear in telemetry
- SET_MODE NOMINAL is accepted
- CPU_OVERLOAD moves the system to DEGRADED_PAYLOAD
- CLEAR_FAULT clears the active fault field
