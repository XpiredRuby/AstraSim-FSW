#!/usr/bin/env python3
"""Run an AstraSim-FSW YAML command/telemetry scenario."""

from __future__ import annotations

import argparse
import socket
import subprocess
import sys
import threading
import time
from pathlib import Path
from typing import Any

import yaml

REPO_ROOT = Path(__file__).resolve().parents[1]
TOOLS_DIR = REPO_ROOT / "tools"

sys.path.insert(0, str(TOOLS_DIR))

from telemetry_receiver import decode_packet  # noqa: E402


def send_command(host: str, port: int, sequence: int, command: str, argument: str | None) -> None:
    cmd = [
        sys.executable,
        str(TOOLS_DIR / "send_command.py"),
        "--host",
        host,
        "--port",
        str(port),
        "--sequence",
        str(sequence),
        "--command",
        command,
    ]

    if argument is not None:
        cmd.extend(["--argument", argument])

    subprocess.run(cmd, check=True, cwd=REPO_ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)


def telemetry_listener(sock: socket.socket, stop_event: threading.Event, packets: list[dict[str, Any]]) -> None:
    sock.settimeout(0.2)

    while not stop_event.is_set():
        try:
            data, _address = sock.recvfrom(2048)
        except socket.timeout:
            continue

        try:
            packets.append(decode_packet(data))
        except ValueError as error:
            packets.append({"decode_error": str(error)})


def packet_matches(packet: dict[str, Any], expected: dict[str, Any]) -> bool:
    checks = {
        "ack_seq": "last_command_sequence_number",
        "ack_cmd": "last_command_id",
        "ack_status": "last_command_status",
        "mode": "mode",
        "fault": "last_fault",
    }

    for scenario_key, packet_key in checks.items():
        if scenario_key not in expected:
            continue

        if packet.get(packet_key) != expected[scenario_key]:
            return False

    return True


def wait_for_expectation(
    packets: list[dict[str, Any]],
    expected: dict[str, Any],
    timeout_s: float,
) -> dict[str, Any] | None:
    deadline = time.monotonic() + timeout_s
    checked = 0

    while time.monotonic() < deadline:
        for packet in packets[checked:]:
            if "decode_error" not in packet and packet_matches(packet, expected):
                return packet

        checked = len(packets)
        time.sleep(0.05)

    return None


def write_report(
    scenario_name: str,
    passed: bool,
    packets: list[dict[str, Any]],
    server_output: str,
    scenario: dict[str, Any],
) -> Path:
    reports_dir = REPO_ROOT / "reports"
    reports_dir.mkdir(exist_ok=True)

    report_path = reports_dir / f"scenario_{scenario_name}_output.txt"

    lines: list[str] = []
    lines.append(f"# Scenario Output: {scenario_name}")
    lines.append("")
    lines.append(f"Result: {'PASS' if passed else 'FAIL'}")
    lines.append("")
    lines.append("## Scenario")
    lines.append("")
    lines.append("```yaml")
    lines.append(yaml.safe_dump(scenario, sort_keys=False).rstrip())
    lines.append("```")
    lines.append("")
    lines.append("## Telemetry Packets")
    lines.append("")
    lines.append("```text")

    for packet in packets:
        if "decode_error" in packet:
            lines.append(f"decode_error={packet['decode_error']}")
            continue

        lines.append(
            "seq={sequence_number} t_ms={timestamp_ms} mode={mode} fault={last_fault} "
            "ack_seq={last_command_sequence_number} ack_cmd={last_command_id} "
            "ack_status={last_command_status} cpu={cpu_load_percent:.2f}% "
            "mem={memory_load_percent:.2f}% hb={heartbeat_count}".format(**packet)
        )

    lines.append("```")
    lines.append("")
    lines.append("## C++ Server Output")
    lines.append("")
    lines.append("```text")
    lines.append(server_output.rstrip())
    lines.append("```")
    lines.append("")

    report_path.write_text("\n".join(lines), encoding="utf-8")
    return report_path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("scenario", help="Path to YAML scenario file")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--expect-timeout", type=float, default=2.5)
    args = parser.parse_args()

    scenario_path = Path(args.scenario)
    scenario = yaml.safe_load(scenario_path.read_text(encoding="utf-8"))

    scenario_name = scenario.get("name", scenario_path.stem)
    command_port = int(scenario.get("command_port", 6000))
    telemetry_port = int(scenario.get("telemetry_port", 5005))
    loop_count = int(scenario.get("loop_count", 22))
    sensor_timeout_loop = scenario.get("sensor_timeout_loop")
    watchdog_timeout_loop = scenario.get("watchdog_timeout_loop")
    steps = scenario.get("steps", [])

    server_exe = REPO_ROOT / "build" / "astra_fsw_command_telemetry_demo"
    if not server_exe.exists():
        print("ERROR: build/astra_fsw_command_telemetry_demo does not exist.")
        print("Run: bash ci/run_local_tests.sh")
        return 1

    packets: list[dict[str, Any]] = []
    stop_event = threading.Event()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((args.host, telemetry_port))

    listener = threading.Thread(target=telemetry_listener, args=(sock, stop_event, packets), daemon=True)
    listener.start()

    server_args = [
        str(server_exe),
        str(command_port),
        args.host,
        str(telemetry_port),
        str(loop_count),
    ]

    if sensor_timeout_loop is not None or watchdog_timeout_loop is not None:
        server_args.append(str(int(sensor_timeout_loop or 0)))

    if watchdog_timeout_loop is not None:
        server_args.append(str(int(watchdog_timeout_loop)))

    server = subprocess.Popen(
        server_args,
        cwd=REPO_ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )

    passed = True
    sequence = 1

    try:
        time.sleep(0.8)

        print(f"Running scenario: {scenario_name}")

        for step in steps:
            name = step.get("name", f"step {sequence}")
            command = step.get("command")
            argument = step.get("argument")
            expected = step.get("expect", {})

            if command is not None:
                print(f"[SEND] seq={sequence} {command} {argument or ''}".rstrip())
                send_command(args.host, command_port, sequence, command, argument)
                sequence += 1
            else:
                print(f"[WAIT] {name}")

            matched = wait_for_expectation(packets, expected, args.expect_timeout)

            if matched is None:
                print(f"[FAIL] {name}")
                print(f"Expected: {expected}")
                passed = False
                break

            print(
                f"[PASS] {name}: "
                f"mode={matched['mode']} fault={matched['last_fault']} "
                f"ack_seq={matched['last_command_sequence_number']} "
                f"ack_cmd={matched['last_command_id']} "
                f"ack_status={matched['last_command_status']}"
            )

        try:
            server.wait(timeout=6)
        except subprocess.TimeoutExpired:
            server.terminate()
            server.wait(timeout=2)

    finally:
        stop_event.set()
        listener.join(timeout=1)
        sock.close()

        if server.poll() is None:
            server.terminate()
            server.wait(timeout=2)

    server_output = ""
    if server.stdout is not None:
        server_output = server.stdout.read()

    report_path = write_report(scenario_name, passed, packets, server_output, scenario)

    if passed:
        print("Scenario passed.")
    else:
        print("Scenario failed.")

    print(f"Report written to {report_path.relative_to(REPO_ROOT)}")
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
