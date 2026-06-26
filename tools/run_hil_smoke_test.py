#!/usr/bin/env python3
"""Run the AstraSim-FSW HIL-style command/telemetry smoke test."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SCENARIO = REPO_ROOT / "scenarios" / "hil_smoke_test.yaml"
RUN_SCENARIO = REPO_ROOT / "tools" / "run_scenario.py"


def main() -> int:
    print("== Running HIL smoke test ==")

    result = subprocess.run(
        [sys.executable, str(RUN_SCENARIO), str(SCENARIO)],
        cwd=REPO_ROOT,
        text=True,
    )

    if result.returncode != 0:
        print("ERROR: HIL smoke test failed.")
        return result.returncode

    print("HIL smoke test passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
