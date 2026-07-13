#!/usr/bin/env python3
"""Send one AstraSim-FSW UDP command packet.

Examples:
    python3 tools/send_command.py --command NOOP
    python3 tools/send_command.py --command SET_MODE --argument NOMINAL
    python3 tools/send_command.py --command INJECT_FAULT --argument CPU_OVERLOAD
"""

from __future__ import annotations

import argparse
import socket
import struct
import time


COMMAND_MAGIC = 0x434D4453
COMMAND_VERSION = 1

COMMANDS = {
    "NOOP": 0,
    "SET_MODE": 1,
    "INJECT_FAULT": 2,
    "REQUEST_TELEMETRY": 3,
    "CLEAR_FAULT": 4,
}

MODES = {
    "BOOT": 0,
    "NOMINAL": 1,
    "DEGRADED_SENSOR": 2,
    "DEGRADED_PAYLOAD": 3,
    "SAFE": 4,
    "RECOVERY": 5,
    "STANDBY": 6,
    "TEST": 7,
}

FAULTS = {
    "NONE": 0,
    "SENSOR_TIMEOUT": 100,
    "SENSOR_INVALID_DATA": 101,
    "PAYLOAD_HEARTBEAT_TIMEOUT": 200,
    "CPU_OVERLOAD": 300,
    "MEMORY_OVERLOAD": 301,
    "TELEMETRY_SOCKET_FAILURE": 400,
    "COMMAND_BAD_CRC": 500,
    "COMMAND_UNKNOWN_ID": 501,
    "COMMAND_TIMEOUT": 502,
    "WATCHDOG_DEADLINE_MISS": 600,
}


def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF

    for byte in data:
        crc ^= byte << 8

        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF

    return crc


def parse_argument(command: str, argument: str | None) -> int:
    if command == "SET_MODE":
        if argument is None:
            raise ValueError("SET_MODE requires --argument MODE")
        return MODES[argument]

    if command == "INJECT_FAULT":
        if argument is None:
            raise ValueError("INJECT_FAULT requires --argument FAULT")
        return FAULTS[argument]

    return 0


def build_packet(
    sequence: int,
    command: str,
    argument_value: int,
    timestamp_ms: int | None = None,
) -> bytes:
    if timestamp_ms is None:
        timestamp_ms = int(time.time() * 1000)

    command_id = COMMANDS[command]
    payload = struct.pack(
        "<IHIQHI",
        COMMAND_MAGIC,
        COMMAND_VERSION,
        sequence,
        timestamp_ms,
        command_id,
        argument_value,
    )

    crc = crc16_ccitt(payload)
    return payload + struct.pack("<H", crc)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=6000)
    parser.add_argument("--sequence", type=int, default=1)
    parser.add_argument("--timestamp-ms", type=int)
    parser.add_argument("--command", choices=COMMANDS.keys(), required=True)
    parser.add_argument("--argument")
    args = parser.parse_args()

    argument_value = parse_argument(args.command, args.argument)
    packet = build_packet(
        args.sequence,
        args.command,
        argument_value,
        timestamp_ms=args.timestamp_ms,
    )

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(packet, (args.host, args.port))
    finally:
        sock.close()

    print(
        f"Sent command {args.command} "
        f"argument={args.argument or argument_value} "
        f"seq={args.sequence} "
        f"timestamp_ms={args.timestamp_ms if args.timestamp_ms is not None else 'now'} "
        f"to {args.host}:{args.port}"
    )


if __name__ == "__main__":
    main()
