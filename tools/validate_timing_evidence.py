#!/usr/bin/env python3
"""Validate ASTRA-OS host timing evidence without imposing a performance claim."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[1]

REQUIRED_NUMERIC_FIELDS = (
    "ticks",
    "mission_step_ms",
    "overload_every",
    "total_elapsed_ns",
    "mean_step_ns",
    "min_step_ns",
    "p50_step_ns",
    "p95_step_ns",
    "p99_step_ns",
    "max_step_ns",
    "scheduler_deadline_misses",
)


def validate(payload: dict[str, Any], *, expect_misses: bool | None) -> list[str]:
    problems: list[str] = []

    if payload.get("schema") != "astra-os.host-timing.v1":
        problems.append("unexpected or missing schema")

    if not isinstance(payload.get("claim_boundary"), str) or not payload["claim_boundary"]:
        problems.append("missing claim boundary")

    for field in REQUIRED_NUMERIC_FIELDS:
        value = payload.get(field)
        if not isinstance(value, (int, float)) or isinstance(value, bool):
            problems.append(f"{field} must be numeric")
        elif value < 0:
            problems.append(f"{field} must not be negative")

    ticks = payload.get("ticks")
    if isinstance(ticks, int) and ticks <= 0:
        problems.append("ticks must be greater than zero")

    ordered_fields = (
        "min_step_ns",
        "p50_step_ns",
        "p95_step_ns",
        "p99_step_ns",
        "max_step_ns",
    )
    ordered_values = [payload.get(field) for field in ordered_fields]
    if all(isinstance(value, (int, float)) for value in ordered_values):
        if ordered_values != sorted(ordered_values):
            problems.append("timing percentile fields are not monotonic")

    if payload.get("final_mode") != "NOMINAL":
        problems.append("timing campaign did not finish in NOMINAL")
    if payload.get("final_fault") != "NONE":
        problems.append("timing campaign finished with an unexpected fault")

    misses = payload.get("scheduler_deadline_misses")
    if isinstance(misses, int) and expect_misses is True and misses <= 0:
        problems.append("controlled-overrun campaign produced no deadline misses")
    if isinstance(misses, int) and expect_misses is False and misses != 0:
        problems.append("nominal campaign produced scheduler deadline misses")

    return problems


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("evidence", type=Path)
    expectation = parser.add_mutually_exclusive_group()
    expectation.add_argument("--expect-misses", action="store_true")
    expectation.add_argument("--expect-no-misses", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    evidence = args.evidence if args.evidence.is_absolute() else REPO_ROOT / args.evidence

    try:
        payload = json.loads(evidence.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        print(f"ERROR: {error}")
        return 1

    expectation: bool | None = None
    if args.expect_misses:
        expectation = True
    elif args.expect_no_misses:
        expectation = False

    problems = validate(payload, expect_misses=expectation)
    if problems:
        for problem in problems:
            print(f"FAIL: {problem}")
        return 1

    print(f"PASS: timing evidence valid: {evidence}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
