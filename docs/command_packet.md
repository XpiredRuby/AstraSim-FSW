# Command Packet

AstraSim-FSW command packets represent ground-to-flight-software commands.

## Purpose

The command packet module is the opposite side of telemetry:

- Telemetry: flight software sends state to ground
- Command: ground sends instructions to flight software

## Supported Commands

| Command | Purpose | Argument |
|---|---|---|
| `NOOP` | No operation / link test | `0` |
| `SET_MODE` | Request a mode transition | Desired `Mode` numeric value |
| `INJECT_FAULT` | Inject a simulated fault | Desired `FaultCode` numeric value |
| `REQUEST_TELEMETRY` | Request immediate telemetry | `0` |
| `CLEAR_FAULT` | Clear active fault state | `0` |

## Validation

Each command packet includes:

- Magic value
- Version
- Sequence number
- Timestamp
- Command ID
- Argument
- CRC-16-CCITT

Invalid packet size, bad CRC, unknown command ID, invalid mode argument, or invalid fault argument must be rejected.
