#!/usr/bin/env python3
"""Run all AstraSim-FSW YAML scenarios."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SCENARIOS_DIR = REPO_ROOT / "scenarios"
RUN_SCENARIO = REPO_ROOT / "tools" / "run_scenario.py"
CHECK_REQUIREMENTS = REPO_ROOT / "tools" / "check_requirements.py"
RUN_MONTE_CARLO = REPO_ROOT / "tools" / "run_monte_carlo.py"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--pattern",
        default="*.yaml",
        help="Scenario file glob pattern inside scenarios/.",
    )
    parser.add_argument(
        "--skip-build",
        action="store_true",
        help="Skip ci/run_local_tests.sh before running scenarios.",
    )
    parser.add_argument(
        "--skip-requirements",
        action="store_true",
        help="Skip requirement traceability check after scenarios.",
    )
    parser.add_argument(
        "--skip-monte-carlo",
        action="store_true",
        help="Skip Monte Carlo regression after deterministic scenarios.",
    )
    parser.add_argument(
        "--monte-carlo-trials",
        type=int,
        default=25,
        help="Number of Monte Carlo trials to run.",
    )
    parser.add_argument(
        "--monte-carlo-seed",
        type=int,
        default=20260626,
        help="Seed for reproducible Monte Carlo verification.",
    )
    args = parser.parse_args()

    scenario_files = sorted(SCENARIOS_DIR.glob(args.pattern))

    if not scenario_files:
        print(f"ERROR: no scenarios found for pattern {args.pattern!r}")
        return 1

    if not RUN_SCENARIO.exists():
        print("ERROR: tools/run_scenario.py not found.")
        return 1

    if not args.skip_build:
        print("== Building and running unit tests ==")
        build = subprocess.run(
            ["bash", "ci/run_local_tests.sh"],
            cwd=REPO_ROOT,
            text=True,
        )
        if build.returncode != 0:
            print("ERROR: build/tests failed.")
            return build.returncode

    print()
    print("== Running YAML scenarios ==")

    results: list[tuple[Path, int]] = []

    for scenario in scenario_files:
        print()
        print(f"--- {scenario.relative_to(REPO_ROOT)} ---")
        result = subprocess.run(
            [sys.executable, str(RUN_SCENARIO), str(scenario)],
            cwd=REPO_ROOT,
            text=True,
        )
        results.append((scenario, result.returncode))

    print()
    print("== Scenario Summary ==")

    passed = 0
    failed = 0

    for scenario, returncode in results:
        if returncode == 0:
            passed += 1
            status = "PASS"
        else:
            failed += 1
            status = "FAIL"

        print(f"{status}: {scenario.relative_to(REPO_ROOT)}")

    print()
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")
    print(f"Total:  {len(results)}")

    if failed != 0:
        return 1

    if not args.skip_monte_carlo:
        print()
        print("== Running Monte Carlo regression ==")

        monte_carlo_result = subprocess.run(
            [
                sys.executable,
                str(RUN_MONTE_CARLO),
                "--trials",
                str(args.monte_carlo_trials),
                "--seed",
                str(args.monte_carlo_seed),
            ],
            cwd=REPO_ROOT,
            text=True,
        )

        if monte_carlo_result.returncode != 0:
            print("ERROR: Monte Carlo regression failed.")
            return monte_carlo_result.returncode

    if not args.skip_requirements:
        print()
        print("== Checking requirement traceability ==")

        req_result = subprocess.run(
            [sys.executable, str(CHECK_REQUIREMENTS)],
            cwd=REPO_ROOT,
            text=True,
        )

        if req_result.returncode != 0:
            print("ERROR: requirement traceability check failed.")
            return req_result.returncode

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
