#!/usr/bin/env python3
"""Generate deterministic binary seeds for the command-packet fuzzer."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import struct
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = REPO_ROOT / "build-fuzz" / "command-corpus"
COMMAND_MAGIC = 0x434D4453
COMMAND_VERSION = 1


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


def command_packet(
    *,
    sequence: int,
    timestamp_ms: int,
    command_id: int,
    argument: int,
    magic: int = COMMAND_MAGIC,
    version: int = COMMAND_VERSION,
    valid_crc: bool = True,
) -> bytes:
    without_crc = struct.pack(
        "<IHIQHI",
        magic,
        version,
        sequence,
        timestamp_ms,
        command_id,
        argument,
    )
    crc = crc16_ccitt(without_crc)
    if not valid_crc:
        crc ^= 0x0001
    return without_crc + struct.pack("<H", crc)


def seed_cases() -> list[tuple[str, bytes, str]]:
    valid_noop = command_packet(sequence=0, timestamp_ms=0, command_id=0, argument=0)
    valid_set_mode = command_packet(sequence=1, timestamp_ms=1000, command_id=1, argument=1)
    valid_fault = command_packet(sequence=2, timestamp_ms=2000, command_id=2, argument=300)
    valid_request = command_packet(sequence=3, timestamp_ms=3000, command_id=3, argument=0)
    valid_clear = command_packet(sequence=4, timestamp_ms=4000, command_id=4, argument=0)

    return [
        ("empty", b"", "empty input"),
        ("one_byte", b"\x00", "minimal malformed input"),
        ("valid_noop", valid_noop, "valid NOOP packet"),
        ("valid_set_mode", valid_set_mode, "valid SET_MODE NOMINAL packet"),
        ("valid_inject_fault", valid_fault, "valid INJECT_FAULT CPU_OVERLOAD packet"),
        ("valid_request_telemetry", valid_request, "valid REQUEST_TELEMETRY packet"),
        ("valid_clear_fault", valid_clear, "valid CLEAR_FAULT packet"),
        ("bad_crc", command_packet(sequence=5, timestamp_ms=5000, command_id=0, argument=0, valid_crc=False), "fixed-size packet with invalid CRC"),
        ("bad_magic", command_packet(sequence=6, timestamp_ms=6000, command_id=0, argument=0, magic=0xDEADBEEF), "valid CRC but invalid magic"),
        ("bad_version", command_packet(sequence=7, timestamp_ms=7000, command_id=0, argument=0, version=2), "valid CRC but unsupported version"),
        ("bad_command_id", command_packet(sequence=8, timestamp_ms=8000, command_id=99, argument=0), "valid CRC but unknown command ID"),
        ("truncated_valid", valid_set_mode[:-1], "valid packet truncated by one byte"),
        ("oversized_valid", valid_set_mode + b"\x00", "valid packet with one trailing byte"),
        ("all_ff", bytes([0xFF] * 26), "fixed-size high-value input"),
        ("large_input", bytes(range(64)), "input above protocol length"),
    ]


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output = args.output if args.output.is_absolute() else REPO_ROOT / args.output
    corpus = output / "corpus"

    if corpus.exists():
        shutil.rmtree(corpus)
    corpus.mkdir(parents=True, exist_ok=True)

    manifest: dict[str, Any] = {
        "schema": "astra-os.command-fuzz-corpus.v1",
        "command_magic": f"0x{COMMAND_MAGIC:08X}",
        "command_version": COMMAND_VERSION,
        "corpus_directory": "corpus",
        "seeds": [],
    }

    for name, data, description in seed_cases():
        path = corpus / name
        path.write_bytes(data)
        manifest["seeds"].append(
            {
                "name": name,
                "description": description,
                "size_bytes": len(data),
                "sha256": sha256(data),
            }
        )

    manifest_path = output / "manifest.json"
    manifest_path.write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    print(f"Generated {len(manifest['seeds'])} command fuzz seeds in {corpus}")
    print(f"Manifest: {manifest_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
