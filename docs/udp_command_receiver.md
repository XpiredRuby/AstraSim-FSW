# UDP Command Receiver

The UDP command receiver listens for ground-to-flight-software command packets.

## Purpose

This module receives binary command packets over UDP and validates them using the `CommandPacket` decoder.

It does not directly change flight software state. State changes are handled by the `CommandProcessor`.

## Current Behavior

- Creates a UDP socket
- Binds to a requested IP and port
- Receives raw UDP datagrams
- Deserializes valid command packets
- Rejects malformed packets, corrupted packets, and invalid CRC packets

## Python Command Sender

A ground-side helper script is included:

```bash
python3 tools/send_command.py --host 127.0.0.1 --port 6000 --command SET_MODE --argument NOMINAL
```

Example commands:

```bash
python3 tools/send_command.py --command NOOP
python3 tools/send_command.py --command SET_MODE --argument NOMINAL
python3 tools/send_command.py --command INJECT_FAULT --argument CPU_OVERLOAD
python3 tools/send_command.py --command CLEAR_FAULT
```

## Future Use

A future flight software command server will combine:

- `UdpCommandReceiver`
- `CommandPacket`
- `CommandProcessor`
- `ModeManager`
- `UdpTelemetrySender`

That will allow a laptop to command the flight software over UDP while telemetry is streamed back.
