#!/usr/bin/env python3

from __future__ import annotations

import argparse
import random
import subprocess
import sys
import tempfile
import textwrap
import time
from pathlib import Path

import yaml


REPO_ROOT = Path(__file__).resolve().parents[1]
RUN_SCENARIO = REPO_ROOT / "tools" / "run_scenario.py"
REPORTS_DIR = REPO_ROOT / "reports"


def make_case(rng: random.Random, trial_id: int) -> dict:
    command_port = rng.randint(20000, 50000)
    telemetry_port = command_port + 1

    templates = [
        {
            "suffix": "nominal_transition",
            "description": "Randomized verification of BOOT to NOMINAL transition.",
            "loop_count": 12,
            "requirements": ["FSW-REQ-002", "FSW-REQ-106", "VER-REQ-006"],
            "steps": [
                {
                    "name": "set nominal",
                    "command": "SET_MODE",
                    "argument": "NOMINAL",
                    "expect": {
                        "ack_seq": 1,
                        "ack_cmd": "SET_MODE",
                        "ack_status": 0,
                        "mode": "NOMINAL",
                        "fault": "NONE",
                    },
                }
            ],
        },
        {
            "suffix": "invalid_transition",
            "description": "Randomized verification of invalid transition rejection.",
            "loop_count": 12,
            "requirements": ["FSW-REQ-003", "FSW-REQ-106", "VER-REQ-006"],
            "steps": [
                {
                    "name": "reject boot to recovery",
                    "command": "SET_MODE",
                    "argument": "RECOVERY",
                    "expect": {
                        "ack_seq": 1,
                        "ack_cmd": "SET_MODE",
                        "ack_status": 2,
                        "mode": "BOOT",
                        "fault": "NONE",
                    },
                }
            ],
        },
        {
            "suffix": "cpu_fault_clear",
            "description": "Randomized verification of CPU fault and fault clearing.",
            "loop_count": 22,
            "requirements": [
                "FSW-REQ-002",
                "FSW-REQ-004",
                "FSW-REQ-005",
                "FSW-REQ-106",
                "VER-REQ-006",
            ],
            "steps": [
                {
                    "name": "set nominal",
                    "command": "SET_MODE",
                    "argument": "NOMINAL",
                    "expect": {
                        "ack_seq": 1,
                        "ack_cmd": "SET_MODE",
                        "ack_status": 0,
                        "mode": "NOMINAL",
                        "fault": "NONE",
                    },
                },
                {
                    "name": "inject cpu fault",
                    "command": "INJECT_FAULT",
                    "argument": "CPU_OVERLOAD",
                    "expect": {
                        "ack_seq": 2,
                        "ack_cmd": "INJECT_FAULT",
                        "ack_status": 0,
                        "mode": "DEGRADED_PAYLOAD",
                        "fault": "CPU_OVERLOAD",
                    },
                },
                {
                    "name": "clear fault",
                    "command": "CLEAR_FAULT",
                    "expect": {
                        "ack_seq": 3,
                        "ack_cmd": "CLEAR_FAULT",
                        "ack_status": 0,
                        "mode": "DEGRADED_PAYLOAD",
                        "fault": "NONE",
                    },
                },
            ],
        },
        {
            "suffix": "sensor_timeout",
            "description": "Randomized verification of sensor timeout forcing SAFE mode.",
            "loop_count": 22,
            "sensor_timeout_loop": rng.randint(7, 11),
            "requirements": ["FSW-REQ-002", "FSW-REQ-006", "FSW-REQ-106", "VER-REQ-006"],
            "steps": [
                {
                    "name": "set nominal before sensor timeout",
                    "command": "SET_MODE",
                    "argument": "NOMINAL",
                    "expect": {
                        "ack_seq": 1,
                        "ack_cmd": "SET_MODE",
                        "ack_status": 0,
                        "mode": "NOMINAL",
                        "fault": "NONE",
                    },
                },
                {
                    "name": "wait for sensor timeout safe mode",
                    "expect": {
                        "mode": "SAFE",
                        "fault": "SENSOR_TIMEOUT",
                    },
                },
            ],
        },
        {
            "suffix": "watchdog_timeout",
            "description": "Randomized verification of watchdog timeout forcing SAFE mode.",
            "loop_count": 22,
            "watchdog_timeout_loop": rng.randint(7, 11),
            "requirements": ["FSW-REQ-002", "FSW-REQ-007", "FSW-REQ-106", "VER-REQ-006"],
            "steps": [
                {
                    "name": "set nominal before watchdog timeout",
                    "command": "SET_MODE",
                    "argument": "NOMINAL",
                    "expect": {
                        "ack_seq": 1,
                        "ack_cmd": "SET_MODE",
                        "ack_status": 0,
                        "mode": "NOMINAL",
                        "fault": "NONE",
                    },
                },
                {
                    "name": "wait for watchdog timeout safe mode",
                    "expect": {
                        "mode": "SAFE",
                        "fault": "WATCHDOG_DEADLINE_MISS",
                    },
                },
            ],
        },
    ]

    case = rng.choice(templates)
    scenario = {
        "name": f"monte_carlo_trial_{trial_id:03d}_{case['suffix']}",
        "description": case["description"],
        "requirements": case["requirements"],
        "command_port": command_port,
        "telemetry_port": telemetry_port,
        "loop_count": case["loop_count"],
        "steps": case["steps"],
    }

    if "sensor_timeout_loop" in case:
        scenario["sensor_timeout_loop"] = case["sensor_timeout_loop"]

    if "watchdog_timeout_loop" in case:
        scenario["watchdog_timeout_loop"] = case["watchdog_timeout_loop"]

    return scenario


def scenario_report_path(name: str) -> Path:
    return REPORTS_DIR / f"scenario_{name}_output.txt"


def run_trial(scenario: dict, build_dir: str = "build") -> tuple[bool, str]:
    with tempfile.NamedTemporaryFile("w", suffix=".yaml", delete=False) as f:
        yaml.safe_dump(scenario, f, sort_keys=False)
        scenario_path = Path(f.name)

    try:
        result = subprocess.run(
            [
                sys.executable,
                str(RUN_SCENARIO),
                str(scenario_path),
                "--build-dir",
                build_dir,
            ],
            cwd=REPO_ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=20,
        )
        return result.returncode == 0, result.stdout
    finally:
        scenario_path.unlink(missing_ok=True)
        scenario_report_path(scenario["name"]).unlink(missing_ok=True)


def write_report(seed: int, trials: list[tuple[dict, bool, str]], output_path: Path) -> None:
    passed = sum(1 for _, ok, _ in trials if ok)
    failed = len(trials) - passed

    lines = [
        "# Monte Carlo Regression Report",
        "",
        f"Seed: `{seed}`",
        f"Trials: `{len(trials)}`",
        f"Passed: `{passed}`",
        f"Failed: `{failed}`",
        f"Result: {'PASS' if failed == 0 else 'FAIL'}",
        "",
        "## Trial Summary",
        "",
        "| Trial | Scenario Type | Result | Requirements |",
        "|---|---|---|---|",
    ]

    for scenario, ok, _ in trials:
        scenario_type = scenario["name"].replace("monte_carlo_trial_", "", 1)
        result = "PASS" if ok else "FAIL"
        reqs = ", ".join(scenario["requirements"])
        lines.append(f"| {scenario['name']} | {scenario_type} | {result} | {reqs} |")

    failures = [(scenario, output) for scenario, ok, output in trials if not ok]
    if failures:
        lines.extend(["", "## Failure Logs", ""])
        for scenario, output in failures:
            lines.append(f"### {scenario['name']}")
            lines.append("")
            lines.append("```text")
            lines.append(output.rstrip())
            lines.append("```")
            lines.append("")

    output_path.parent.mkdir(exist_ok=True)
    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--trials", type=int, default=25)
    parser.add_argument("--seed", type=int, default=int(time.time()))
    parser.add_argument("--output", default="reports/monte_carlo_report.md")
    parser.add_argument(
        "--build-dir",
        default="build",
        help="Directory containing the built ASTRA-OS executables.",
    )
    args = parser.parse_args()

    if args.trials <= 0:
        raise SystemExit("ERROR: --trials must be positive")

    rng = random.Random(args.seed)
    trials: list[tuple[dict, bool, str]] = []

    print(f"Monte Carlo seed: {args.seed}")
    print(f"Trials: {args.trials}")

    for trial_id in range(1, args.trials + 1):
        scenario = make_case(rng, trial_id)
        ok, output = run_trial(scenario, args.build_dir)
        trials.append((scenario, ok, output))

        result = "PASS" if ok else "FAIL"
        print(f"[{result}] {scenario['name']}")

        if not ok:
            print(textwrap.indent(output.rstrip(), "  "))

    output_path = REPO_ROOT / args.output
    write_report(args.seed, trials, output_path)

    passed = sum(1 for _, ok, _ in trials if ok)
    failed = len(trials) - passed

    print()
    print("== Monte Carlo Summary ==")
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")
    print(f"Total:  {len(trials)}")
    print(f"Report written to {output_path.relative_to(REPO_ROOT)}")

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
