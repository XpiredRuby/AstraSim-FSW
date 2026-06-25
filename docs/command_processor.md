# Command Processor

The command processor applies decoded ground commands to the flight software state machine.

## Responsibilities

- Accept decoded `CommandPacket` objects.
- Apply valid commands to the `ModeManager`.
- Reject invalid command arguments.
- Reject invalid mode transitions.
- Track the latest injected fault.
- Return a structured `CommandResult`.

## Current Command Behavior

| Command | Behavior |
|---|---|
| `NOOP` | Accepted. No state change. |
| `REQUEST_TELEMETRY` | Accepted. Future versions will trigger immediate telemetry output. |
| `CLEAR_FAULT` | Clears the stored fault state. |
| `SET_MODE` | Converts argument to `Mode` and requests a mode transition. |
| `INJECT_FAULT` | Converts argument to `FaultCode`, stores it, and sends it to `ModeManager::handle_fault`. |

## Rejection Cases

The command processor rejects:

- Invalid mode arguments
- Invalid fault arguments
- Invalid mode transitions

This creates a clean boundary between raw packet decoding and flight software state control.
