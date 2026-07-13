# Raspberry Pi Deployment

ASTRA-OS can be packaged for deployment to a Raspberry Pi target. The package contains the native flight-software binaries, ground command/telemetry tools, scenarios, the protocol manifest, requirements, and the verification matrix.

## Build from an explicit directory

For the verified native Pi build:

```bash
bash tools/package_pi_deployment.sh --build-dir build-pi
```

`--build-dir` may be relative to the repository root or absolute. The packaging script configures and builds the selected directory before assembling the archive, so the source of the packaged binaries is explicit.

The archive is written to:

```text
dist/astrasim-fsw-pi.tar.gz
```

The package report is written to:

```text
reports/pi_deployment_package_report.md
```

## Copy package to a Raspberry Pi

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

Terminal 1 runs the packaged command/telemetry executable with the desired command and telemetry endpoints.

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

- command acknowledgements appear in telemetry;
- `SET_MODE NOMINAL` is accepted;
- `CPU_OVERLOAD` moves the system to `DEGRADED_PAYLOAD`;
- `CLEAR_FAULT` clears the active fault field.

## Claim boundary

Package generation proves that a reproducible target bundle can be assembled. It does not independently prove physical target execution, certification, flight qualification, hard-real-time behavior, or spacecraft-hardware compatibility. Native Pi execution evidence is documented separately in `reports/ASTRA_OS_RASPBERRY_PI_VERIFICATION_REPORT.md`.
