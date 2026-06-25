# Command / Telemetry Demo

This demo connects the command side and telemetry side of AstraSim-FSW.

The executable now uses `FlightSoftwareApp` as the central flight-software core loop.

## What It Demonstrates

- Python sends UDP command packets.
- C++ receives and decodes command packets.
- `FlightSoftwareApp` processes commands through `CommandProcessor`.
- `CommandProcessor` applies valid commands to `ModeManager`.
- `FlightSoftwareApp` generates telemetry from current state.
- C++ sends telemetry packets back over UDP.
- Python receiver decodes the updated flight software state.

## Manual Demo

### New terminal: telemetry receiver

```bash
cd ~/projects/AstraSim-FSW
python3 tools/telemetry_receiver.py --port 5005
```

### Old terminal: command/telemetry demo server

```bash
cd ~/projects/AstraSim-FSW
./build/astra_fsw_command_telemetry_demo 6000 127.0.0.1 5005 30
```

### Third terminal: send commands

Enter WSL first if needed:

```powershell
wsl
```

Then send commands:

```bash
cd ~/projects/AstraSim-FSW
python3 tools/send_command.py --host 127.0.0.1 --port 6000 --sequence 1 --command SET_MODE --argument NOMINAL
python3 tools/send_command.py --host 127.0.0.1 --port 6000 --sequence 2 --command INJECT_FAULT --argument CPU_OVERLOAD
python3 tools/send_command.py --host 127.0.0.1 --port 6000 --sequence 3 --command CLEAR_FAULT
```

## Automated Capture

From the project root:

```bash
tools/run_command_telemetry_demo.sh
```

This starts the Python telemetry receiver, starts the C++ command/telemetry server, sends three Python ground commands, captures all outputs, and writes:

```text
reports/command_telemetry_demo_output.txt
```

## Expected Server Result

```text
RX command seq=1 id=SET_MODE status=ACCEPTED mode=NOMINAL fault=NONE
RX command seq=2 id=INJECT_FAULT status=ACCEPTED mode=DEGRADED_PAYLOAD fault=CPU_OVERLOAD
RX command seq=3 id=CLEAR_FAULT status=ACCEPTED mode=DEGRADED_PAYLOAD fault=NONE
```

## Expected Telemetry Result

The telemetry receiver should show matching state updates:

```text
mode=NOMINAL fault=NONE
mode=DEGRADED_PAYLOAD fault=CPU_OVERLOAD
mode=DEGRADED_PAYLOAD fault=NONE
```
