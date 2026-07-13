#!/usr/bin/env python3
"""Run a deterministic command/telemetry campaign for every supported fault."""

from __future__ import annotations

import argparse
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path

import yaml

REPO_ROOT = Path(__file__).resolve().parents[1]
RUN_SCENARIO = REPO_ROOT / "tools" / "run_scenario.py"
DEFAULT_REPORT = REPO_ROOT / "reports" / "fdir_campaign_report.md"


@dataclass(frozen=True)
class FaultCase:
    fault: str
    expected_mode: str


FAULT_CASES = (
    FaultCase("SENSOR_TIMEOUT", "SAFE"),
    FaultCase("SENSOR_INVALID_DATA", "SAFE"),
    FaultCase("PAYLOAD_HEARTBEAT_TIMEOUT", "DEGRADED_PAYLOAD"),
    FaultCase("CPU_OVERLOAD", "DEGRADED_PAYLOAD"),
    FaultCase("MEMORY_OVERLOAD", "SAFE"),
    FaultCase("TELEMETRY_SOCKET_FAILURE", "SAFE"),
    FaultCase("COMMAND_BAD_CRC", "NOMINAL"),
    FaultCase("COMMAND_UNKNOWN_ID", "NOMINAL"),
    FaultCase("COMMAND_TIMEOUT", "SAFE"),
    FaultCase("WATCHDOG_DEADLINE_MISS", "SAFE"),
)


def scenario_for(case: FaultCase, index: int) -> dict[str, object]:
    name = f"fdir_{case.fault.lower()}"
    return {
        "name": name,
        "description": f"Exercise the explicit FDIR disposition for {case.fault}.",
        "requirements": ["FSW-REQ-207", "FSW-REQ-208", "FSW-REQ-113"],
        "command_port": 6100 + index,
        "telemetry_port": 5100 + index,
        "loop_count": 20,
        "steps": [
            {
                "name": "enter nominal",
                "command": "SET_MODE",
                "argument": "NOMINAL",
                "expect": {
                    "ack_seq": 1,
                    "ack_cmd": "SET_MODE",
                    "ack_status": "ACCEPTED",
                    "mode": "NOMINAL",
                    "fault": "NONE",
                },
            },
            {
                "name": f"inject {case.fault}",
                "command": "INJECT_FAULT",
                "argument": case.fault,
                "expect": {
                    "ack_seq": 2,
                    "ack_cmd": "INJECT_FAULT",
                    "ack_status": "ACCEPTED",
                    "mode": case.expected_mode,
                    "fault": case.fault,
                },
            },
        ],
    }


def write_report(
    output: Path,
    build_dir: str,
    results: list[tuple[FaultCase, str, bool, str]],
) -> None:
    passed = sum(1 for _, _, ok, _ in results if ok)
    failed = len(results) - passed
    lines = [
        "# ASTRA-OS Ten-Case FDIR Campaign",
        "",
        f"Result: {'PASS' if failed == 0 else 'FAIL'}",
        "",
        f"Build directory: `{build_dir}`",
        f"Cases: `{len(results)}`",
        f"Passed: `{passed}`",
        f"Failed: `{failed}`",
        "",
        "The campaign injects each supported fault through the UDP command/telemetry scenario boundary. It verifies the typed acknowledgement, active fault indication, and resulting operational mode. It does not prove coverage of every physical detection mechanism.",
        "",
        "| Fault | Expected mode | Result | Scenario report |",
        "|---|---|---|---|",
    ]

    for case, name, ok, _ in results:
        status = "PASS" if ok else "FAIL"
        lines.append(
            f"| `{case.fault}` | `{case.expected_mode}` | {status} | "
            f"`reports/scenario_{name}_output.txt` |"
        )

    failures = [(case, output_text) for case, _, ok, output_text in results if not ok]
    if failures:
        lines.extend(["", "## Failure logs", ""])
        for case, output_text in failures:
            lines.extend(
                [
                    f"### {case.fault}",
                    "",
                    "```text",
                    output_text.rstrip(),
                    "```",
                    "",
                ]
            )

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", default="build")
    parser.add_argument("--output", type=Path, default=DEFAULT_REPORT)
    args = parser.parse_args()

    output = args.output if args.output.is_absolute() else REPO_ROOT / args.output
    results: list[tuple[FaultCase, str, bool, str]] = []

    print(f"FDIR cases: {len(FAULT_CASES)}")
    print(f"Build directory: {args.build_dir}")

    for index, case in enumerate(FAULT_CASES):
        scenario = scenario_for(case, index)
        name = str(scenario["name"])

        with tempfile.NamedTemporaryFile(
            "w",
            suffix=".yaml",
            encoding="utf-8",
            delete=False,
        ) as handle:
            yaml.safe_dump(scenario, handle, sort_keys=False)
            scenario_path = Path(handle.name)

        try:
            completed = subprocess.run(
                [
                    sys.executable,
                    str(RUN_SCENARIO),
                    str(scenario_path),
                    "--build-dir",
                    args.build_dir,
                ],
                cwd=REPO_ROOT,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=20,
            )
        except subprocess.TimeoutExpired as error:
            output_text = (error.stdout or "") + "\nERROR: scenario timed out"
            results.append((case, name, False, output_text))
            print(f"[FAIL] {case.fault}: timeout")
        else:
            ok = completed.returncode == 0
            results.append((case, name, ok, completed.stdout))
            print(f"[{'PASS' if ok else 'FAIL'}] {case.fault} -> {case.expected_mode}")
        finally:
            scenario_path.unlink(missing_ok=True)

    write_report(output, args.build_dir, results)

    failed = sum(1 for _, _, ok, _ in results if not ok)
    print(f"Report written to {output.relative_to(REPO_ROOT)}")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
