#!/usr/bin/env python3
"""Simple UDP telemetry receiver for AstraSim-FSW.

Usage:
    python3 tools/telemetry_receiver.py --port 5005
"""

import argparse
import socket
import struct


TELEMETRY_MAGIC = 0x41535452
TELEMETRY_VERSION = 1
TELEMETRY_PACKET_TYPE_HEALTH = 1
TELEMETRY_PACKET_SIZE_BYTES = 43

MODES = {
    0: "BOOT",
    1: "NOMINAL",
    2: "DEGRADED_SENSOR",
    3: "DEGRADED_PAYLOAD",
    4: "SAFE",
    5: "RECOVERY",
    6: "STANDBY",
    7: "TEST",
}

FAULTS = {
    0: "NONE",
    100: "SENSOR_TIMEOUT",
    101: "SENSOR_INVALID_DATA",
    200: "PAYLOAD_HEARTBEAT_TIMEOUT",
    300: "CPU_OVERLOAD",
    301: "MEMORY_OVERLOAD",
    400: "TELEMETRY_SOCKET_FAILURE",
    500: "COMMAND_BAD_CRC",
    501: "COMMAND_UNKNOWN_ID",
    502: "COMMAND_TIMEOUT",
    600: "WATCHDOG_DEADLINE_MISS",
}

COMMANDS = {
    0: "NOOP",
    1: "SET_MODE",
    2: "INJECT_FAULT",
    3: "REQUEST_TELEMETRY",
    4: "CLEAR_FAULT",
}

COMMAND_STATUSES = {
    0: "ACCEPTED",
    1: "REJECTED_BAD_ARGUMENT",
    2: "REJECTED_INVALID_TRANSITION",
    3: "REJECTED_UNKNOWN_COMMAND",
    4: "REJECTED_DUPLICATE_SEQUENCE",
    5: "REJECTED_REPLAYED_SEQUENCE",
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


def command_name(command_id: int) -> str:
    if command_id == 0:
        return "NONE"
    return COMMANDS.get(command_id, f"UNKNOWN({command_id})")


def decode_packet(data: bytes) -> dict:
    if len(data) != TELEMETRY_PACKET_SIZE_BYTES:
        raise ValueError(f"wrong packet size: {len(data)} bytes")

    expected_crc = crc16_ccitt(data[:-2])
    actual_crc = struct.unpack_from("<H", data, TELEMETRY_PACKET_SIZE_BYTES - 2)[0]

    if actual_crc != expected_crc:
        raise ValueError(f"bad CRC: expected 0x{expected_crc:04X}, got 0x{actual_crc:04X}")

    (
        magic,
        version,
        packet_type,
        sequence_number,
        timestamp_ms,
        mode,
        last_fault,
        cpu_load_percent,
        memory_load_percent,
        heartbeat_count,
        last_command_sequence_number,
        last_command_id,
        last_command_status,
        _crc,
    ) = struct.unpack("<IHHIQBHffIIBBH", data)

    if magic != TELEMETRY_MAGIC:
        raise ValueError(f"bad magic: 0x{magic:08X}")

    if version != TELEMETRY_VERSION:
        raise ValueError(f"unsupported version: {version}")

    if packet_type != TELEMETRY_PACKET_TYPE_HEALTH:
        raise ValueError(f"unsupported packet type: {packet_type}")

    return {
        "sequence_number": sequence_number,
        "timestamp_ms": timestamp_ms,
        "mode": MODES.get(mode, f"UNKNOWN({mode})"),
        "last_fault": FAULTS.get(last_fault, f"UNKNOWN({last_fault})"),
        "cpu_load_percent": cpu_load_percent,
        "memory_load_percent": memory_load_percent,
        "heartbeat_count": heartbeat_count,
        "last_command_sequence_number": last_command_sequence_number,
        "last_command_id": command_name(last_command_id),
        "last_command_status": COMMAND_STATUSES.get(
            last_command_status,
            f"UNKNOWN({last_command_status})",
        ),
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5005)
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((args.host, args.port))

    print(f"Listening for AstraSim-FSW telemetry on UDP {args.host}:{args.port}")

    while True:
        data, address = sock.recvfrom(2048)

        try:
            packet = decode_packet(data)
        except ValueError as error:
            print(f"{address}: rejected packet: {error}")
            continue

        print(
            f"{address}: "
            f"seq={packet['sequence_number']} "
            f"t_ms={packet['timestamp_ms']} "
            f"mode={packet['mode']} "
            f"fault={packet['last_fault']} "
            f"cpu={packet['cpu_load_percent']:.2f}% "
            f"mem={packet['memory_load_percent']:.2f}% "
            f"hb={packet['heartbeat_count']} "
            f"ack_seq={packet['last_command_sequence_number']} "
            f"ack_cmd={packet['last_command_id']} "
            f"ack_status={packet['last_command_status']}"
        )


if __name__ == "__main__":
    main()
