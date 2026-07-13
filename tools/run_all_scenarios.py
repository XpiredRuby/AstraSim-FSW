#!/usr/bin/env python3
"""Run ASTRA-OS deterministic scenarios and verification checks."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
CI_TESTS = REPO_ROOT / "ci" / "run_local_tests.sh"
SCENARIOS_DIR = REPO_ROOT / "scenarios"
RUN_SCENARIO = REPO_ROOT / "tools" / "run_scenario.py"
CHECK_REQUIREMENTS = REPO_ROOT / "tools" / "check_requirements.py"
CHECK_PROTOCOL = REPO_ROOT / "tools" / "check_protocol_conformance.py"
RUN_MONTE_CARLO = REPO_ROOT / "tools" / "run_monte_carlo.py"
PACKAGE_PI_DEPLOYMENT = REPO_ROOT / "tools" / "package_pi_deployment.sh"


def run_command(command: list[str], title: str) -> int:
    print()
    print(title)

    result = subprocess.run(
        command,
        cwd=REPO_ROOT,
        text=True,
    )

    if result.returncode != 0:
        print(f"ERROR: {title.strip('= ')} failed.")
        return result.returncode

    return 0


def run_build_and_unit_tests() -> int:
    print("== Building and running unit tests ==")

    result = subprocess.run(
        ["bash", str(CI_TESTS)],
        cwd=REPO_ROOT,
        text=True,
    )

    if result.returncode != 0:
        print("ERROR: build/unit tests failed.")
        return result.returncode

    return 0


def scenario_files(pattern: str | None) -> list[Path]:
    if pattern:
        return sorted(SCENARIOS_DIR.glob(pattern))

    return sorted(SCENARIOS_DIR.glob("*.yaml"))


def run_scenarios(pattern: str | None, build_dir: str) -> int:
    print()
    print("== Running YAML scenarios ==")

    scenarios = scenario_files(pattern)

    if not scenarios:
        print("ERROR: no scenarios matched.")
        return 1

    results: list[tuple[Path, int]] = []

    for scenario in scenarios:
        print()
        print(f"--- {scenario.relative_to(REPO_ROOT)} ---")

        result = subprocess.run(
            [
                sys.executable,
                str(RUN_SCENARIO),
                str(scenario),
                "--build-dir",
                build_dir,
            ],
            cwd=REPO_ROOT,
            text=True,
        )

        results.append((scenario, result.returncode))

    passed = sum(1 for _, code in results if code == 0)
    failed = len(results) - passed

    print()
    print("== Scenario Summary ==")

    for scenario, code in results:
        status = "PASS" if code == 0 else "FAIL"
        print(f"{status}: {scenario.relative_to(REPO_ROOT)}")

    print()
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")
    print(f"Total:  {len(results)}")

    return 0 if failed == 0 else 1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--build-dir",
        default="build",
        help="Directory containing the built ASTRA-OS executables.",
    )
    parser.add_argument(
        "--skip-build",
        action="store_true",
        help="Skip ci/run_local_tests.sh before running scenarios.",
    )
    parser.add_argument(
        "--skip-requirements",
        action="store_true",
        help="Skip requirement traceability check after verification.",
    )
    parser.add_argument(
        "--skip-protocol-check",
        action="store_true",
        help="Skip C++/Python protocol manifest conformance checking.",
    )
    parser.add_argument(
        "--skip-monte-carlo",
        action="store_true",
        help="Skip Monte Carlo regression after deterministic scenarios.",
    )
    parser.add_argument(
        "--skip-pi-package",
        action="store_true",
        help="Skip Raspberry Pi deployment package generation.",
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
    parser.add_argument(
        "--pattern",
        help="Only run scenarios matching this glob pattern, for example '*safe_mode.yaml'.",
    )
    args = parser.parse_args()

    if not args.skip_build:
        code = run_build_and_unit_tests()
        if code != 0:
            return code

    code = run_scenarios(args.pattern, args.build_dir)
    if code != 0:
        return code

    if not args.skip_monte_carlo:
        code = run_command(
            [
                sys.executable,
                str(RUN_MONTE_CARLO),
                "--trials",
                str(args.monte_carlo_trials),
                "--seed",
                str(args.monte_carlo_seed),
                "--build-dir",
                args.build_dir,
            ],
            "== Running Monte Carlo regression ==",
        )
        if code != 0:
            return code

    if not args.skip_pi_package:
        code = run_command(
            [
                "bash",
                str(PACKAGE_PI_DEPLOYMENT),
                "--build-dir",
                args.build_dir,
            ],
            "== Building Raspberry Pi deployment package ==",
        )
        if code != 0:
            return code

    if not args.skip_protocol_check:
        code = run_command(
            [sys.executable, str(CHECK_PROTOCOL)],
            "== Checking protocol conformance ==",
        )
        if code != 0:
            return code

    if not args.skip_requirements:
        code = run_command(
            [sys.executable, str(CHECK_REQUIREMENTS)],
            "== Checking requirement traceability ==",
        )
        if code != 0:
            return code

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
