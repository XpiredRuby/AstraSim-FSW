from __future__ import annotations

import subprocess
import unittest
from unittest.mock import patch

from tools.run_monte_carlo import run_trial


class MonteCarloTimeoutTests(unittest.TestCase):
    def test_timeout_is_reported_as_failed_trial(self) -> None:
        scenario = {
            "name": "monte_carlo_timeout_test",
            "requirements": ["VER-REQ-006"],
            "command_port": 6200,
            "telemetry_port": 5200,
            "loop_count": 5,
            "steps": [],
        }

        timeout = subprocess.TimeoutExpired(
            cmd=["python3", "tools/run_scenario.py"],
            timeout=0.25,
            output="partial scenario output",
        )
        with patch("tools.run_monte_carlo.subprocess.run", side_effect=timeout):
            passed, output = run_trial(scenario, timeout_s=0.25)

        self.assertFalse(passed)
        self.assertIn("timed out after 0.2 seconds", output)
        self.assertIn("failed trial rather than crashing", output)
        self.assertIn("partial scenario output", output)


if __name__ == "__main__":
    unittest.main()
