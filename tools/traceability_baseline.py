#!/usr/bin/env python3
"""Shared helpers for ASTRA-OS traceability baselines."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any, Mapping

TRACEABILITY_SCHEMA = "astra-os.traceability-baseline.v1"

CONTROLLED_INTERFACE_PATHS = (
    "config/protocol_manifest.json",
    "fsw/include/astra/command_packet.hpp",
    "fsw/include/astra/command_processor.hpp",
    "fsw/include/astra/mode_manager.hpp",
    "fsw/include/astra/telemetry_packet.hpp",
    "tools/send_command.py",
    "tools/telemetry_receiver.py",
)


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def requirement_fingerprint(requirement: Any) -> str:
    payload = {
        "id": requirement.req_id,
        "text": requirement.text,
        "method": requirement.method,
        "status": requirement.status,
    }
    encoded = json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return sha256_bytes(encoded)


def build_traceability_payload(
    requirements: Mapping[str, Any],
    repo_root: Path,
) -> dict[str, object]:
    interfaces: dict[str, str] = {}
    for relative_path in CONTROLLED_INTERFACE_PATHS:
        path = repo_root / relative_path
        if not path.is_file():
            raise ValueError(f"controlled interface does not exist: {relative_path}")
        interfaces[relative_path] = sha256_file(path)

    return {
        "schema": TRACEABILITY_SCHEMA,
        "requirements": {
            req_id: requirement_fingerprint(requirements[req_id])
            for req_id in sorted(requirements)
        },
        "controlled_interfaces": interfaces,
        "review_note": (
            "Hashes are updated only after requirement and interface changes are reviewed. "
            "A mismatch is treated as pending impact review."
        ),
    }


def read_traceability_payload(path: Path) -> dict[str, object]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise ValueError(f"unable to read traceability baseline {path}: {error}") from error

    if payload.get("schema") != TRACEABILITY_SCHEMA:
        raise ValueError(f"unexpected traceability baseline schema in {path}")
    if not isinstance(payload.get("requirements"), dict):
        raise ValueError(f"traceability baseline requirements must be an object: {path}")
    if not isinstance(payload.get("controlled_interfaces"), dict):
        raise ValueError(f"traceability baseline controlled_interfaces must be an object: {path}")
    return payload
