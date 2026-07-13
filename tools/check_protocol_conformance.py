#!/usr/bin/env python3
"""Verify that C++ and Python protocol definitions match the canonical manifest."""

from __future__ import annotations

import argparse
import importlib.util
import json
import re
import sys
from pathlib import Path
from types import ModuleType
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MANIFEST = REPO_ROOT / "config" / "protocol_manifest.json"
DEFAULT_REPORT = REPO_ROOT / "reports" / "latest" / "protocol_conformance.json"


def load_module(name: str, path: Path) -> ModuleType:
    specification = importlib.util.spec_from_file_location(name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"cannot load module specification for {path}")
    module = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(module)
    return module


def parse_integer(text: str) -> int:
    return int(text, 0)


def parse_cpp_constant(path: Path, name: str) -> int:
    text = path.read_text(encoding="utf-8")
    pattern = re.compile(
        rf"constexpr\s+[^;=]+\s+{re.escape(name)}\s*=\s*(0x[0-9A-Fa-f]+|\d+)\s*;"
    )
    match = pattern.search(text)
    if match is None:
        raise ValueError(f"{path}: constant {name} not found")
    return parse_integer(match.group(1))


def parse_cpp_enum(path: Path, enum_name: str) -> dict[str, int]:
    text = path.read_text(encoding="utf-8")
    enum_pattern = re.compile(
        rf"enum\s+class\s+{re.escape(enum_name)}\s*:\s*[^{{]+\{{(?P<body>.*?)\}}\s*;",
        re.DOTALL,
    )
    match = enum_pattern.search(text)
    if match is None:
        raise ValueError(f"{path}: enum {enum_name} not found")

    entries: dict[str, int] = {}
    for entry in match.group("body").split(","):
        stripped = entry.strip()
        if not stripped:
            continue
        entry_match = re.fullmatch(r"([A-Z][A-Z0-9_]*)\s*=\s*(0x[0-9A-Fa-f]+|\d+)", stripped)
        if entry_match is None:
            raise ValueError(f"{path}: unsupported {enum_name} entry syntax: {stripped!r}")
        entries[entry_match.group(1)] = parse_integer(entry_match.group(2))
    return entries


def invert_integer_map(values: dict[int, str]) -> dict[str, int]:
    return {name: value for value, name in values.items()}


def compare(
    label: str,
    expected: Any,
    actual: Any,
    results: list[dict[str, Any]],
) -> None:
    results.append(
        {
            "label": label,
            "passed": expected == actual,
            "expected": expected,
            "actual": actual,
        }
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    return parser.parse_args()


def resolve(path: Path) -> Path:
    return path if path.is_absolute() else REPO_ROOT / path


def main() -> int:
    args = parse_args()
    manifest_path = resolve(args.manifest)
    report_path = resolve(args.report)
    results: list[dict[str, Any]] = []

    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        mode_header = REPO_ROOT / "fsw" / "include" / "astra" / "mode_manager.hpp"
        command_header = REPO_ROOT / "fsw" / "include" / "astra" / "command_packet.hpp"
        processor_header = REPO_ROOT / "fsw" / "include" / "astra" / "command_processor.hpp"
        telemetry_header = REPO_ROOT / "fsw" / "include" / "astra" / "telemetry_packet.hpp"

        send_command = load_module("astra_send_command", REPO_ROOT / "tools" / "send_command.py")
        telemetry_receiver = load_module(
            "astra_telemetry_receiver",
            REPO_ROOT / "tools" / "telemetry_receiver.py",
        )

        compare("C++ Mode enum", manifest["modes"], parse_cpp_enum(mode_header, "Mode"), results)
        compare("C++ FaultCode enum", manifest["faults"], parse_cpp_enum(mode_header, "FaultCode"), results)
        compare("C++ CommandId enum", manifest["commands"], parse_cpp_enum(command_header, "CommandId"), results)
        compare(
            "C++ CommandStatus enum",
            manifest["command_statuses"],
            parse_cpp_enum(processor_header, "CommandStatus"),
            results,
        )

        compare("C++ command magic", manifest["command_packet"]["magic"], parse_cpp_constant(command_header, "COMMAND_MAGIC"), results)
        compare("C++ command version", manifest["command_packet"]["version"], parse_cpp_constant(command_header, "COMMAND_VERSION"), results)
        compare("C++ command size", manifest["command_packet"]["size_bytes"], parse_cpp_constant(command_header, "COMMAND_PACKET_SIZE_BYTES"), results)
        compare("C++ telemetry magic", manifest["telemetry_packet"]["magic"], parse_cpp_constant(telemetry_header, "TELEMETRY_MAGIC"), results)
        compare("C++ telemetry version", manifest["telemetry_packet"]["version"], parse_cpp_constant(telemetry_header, "TELEMETRY_VERSION"), results)
        compare("C++ telemetry type", manifest["telemetry_packet"]["packet_type_health"], parse_cpp_constant(telemetry_header, "TELEMETRY_PACKET_TYPE_HEALTH"), results)
        compare("C++ telemetry size", manifest["telemetry_packet"]["size_bytes"], parse_cpp_constant(telemetry_header, "TELEMETRY_PACKET_SIZE_BYTES"), results)

        compare("Python sender commands", manifest["commands"], send_command.COMMANDS, results)
        compare("Python sender modes", manifest["modes"], send_command.MODES, results)
        compare("Python sender faults", manifest["faults"], send_command.FAULTS, results)
        compare("Python sender command magic", manifest["command_packet"]["magic"], send_command.COMMAND_MAGIC, results)
        compare("Python sender command version", manifest["command_packet"]["version"], send_command.COMMAND_VERSION, results)

        compare("Python receiver commands", manifest["commands"], invert_integer_map(telemetry_receiver.COMMANDS), results)
        compare("Python receiver modes", manifest["modes"], invert_integer_map(telemetry_receiver.MODES), results)
        compare("Python receiver faults", manifest["faults"], invert_integer_map(telemetry_receiver.FAULTS), results)
        compare("Python receiver statuses", manifest["command_statuses"], invert_integer_map(telemetry_receiver.COMMAND_STATUSES), results)
        compare("Python telemetry magic", manifest["telemetry_packet"]["magic"], telemetry_receiver.TELEMETRY_MAGIC, results)
        compare("Python telemetry version", manifest["telemetry_packet"]["version"], telemetry_receiver.TELEMETRY_VERSION, results)
        compare("Python telemetry type", manifest["telemetry_packet"]["packet_type_health"], telemetry_receiver.TELEMETRY_PACKET_TYPE_HEALTH, results)
        compare("Python telemetry size", manifest["telemetry_packet"]["size_bytes"], telemetry_receiver.TELEMETRY_PACKET_SIZE_BYTES, results)
    except (OSError, ValueError, KeyError, TypeError, RuntimeError, json.JSONDecodeError) as error:
        print(f"ERROR: {error}")
        return 1

    passed = all(result["passed"] for result in results)
    report = {
        "schema": "astra-os.protocol-conformance.v1",
        "manifest": str(manifest_path.relative_to(REPO_ROOT)),
        "passed": passed,
        "check_count": len(results),
        "failed_count": sum(not result["passed"] for result in results),
        "checks": results,
    }
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    for result in results:
        print(f"[{'PASS' if result['passed'] else 'FAIL'}] {result['label']}")
    print(f"Report: {report_path.relative_to(REPO_ROOT)}")

    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
