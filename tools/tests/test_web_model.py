from __future__ import annotations

import json
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
MODEL_PATH = REPO_ROOT / "site" / "model.json"
INDEX_PATH = REPO_ROOT / "site" / "index.html"
APP_PATH = REPO_ROOT / "site" / "app.js"
STYLE_PATH = REPO_ROOT / "site" / "styles.css"


class WebModelTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.model = json.loads(MODEL_PATH.read_text(encoding="utf-8"))

    def test_stable_mode_values(self) -> None:
        expected = {
            "BOOT": 0,
            "NOMINAL": 1,
            "DEGRADED_SENSOR": 2,
            "DEGRADED_PAYLOAD": 3,
            "SAFE": 4,
            "RECOVERY": 5,
            "STANDBY": 6,
            "TEST": 7,
        }
        actual = {item["name"]: item["value"] for item in self.model["modes"]}
        self.assertEqual(actual, expected)

    def test_transition_graph_is_closed_and_reflexive(self) -> None:
        modes = {item["name"] for item in self.model["modes"]}
        transitions = self.model["transitions"]
        self.assertEqual(set(transitions), modes)
        for source, targets in transitions.items():
            self.assertIn(source, targets)
            self.assertTrue(set(targets).issubset(modes))

    def test_fault_set_and_priorities(self) -> None:
        faults = self.model["faults"]
        self.assertEqual(len(faults), 11)
        self.assertEqual(faults["WATCHDOG_DEADLINE_MISS"]["priority"], 100)
        self.assertEqual(faults["CPU_OVERLOAD"]["targetMode"], "DEGRADED_PAYLOAD")
        self.assertEqual(faults["SENSOR_TIMEOUT"]["targetMode"], "SAFE")
        self.assertIsNone(faults["COMMAND_BAD_CRC"]["targetMode"])

    def test_command_guard_and_recovery_bounds(self) -> None:
        self.assertGreater(self.model["guard"]["maximumAgeMs"], 0)
        self.assertGreaterEqual(self.model["guard"]["maximumFutureSkewMs"], 0)
        self.assertEqual(self.model["recoverySupervisor"]["maximumFailedExits"], 3)

    def test_scenarios_reference_known_commands_and_arguments(self) -> None:
        commands = {"NOOP", "SET_MODE", "INJECT_FAULT", "REQUEST_TELEMETRY", "CLEAR_FAULT"}
        modes = set(self.model["transitions"])
        faults = set(self.model["faults"])
        scenario_ids: set[str] = set()
        for scenario in self.model["scenarios"]:
            self.assertNotIn(scenario["id"], scenario_ids)
            scenario_ids.add(scenario["id"])
            self.assertTrue(scenario["steps"])
            for step in scenario["steps"]:
                if "command" not in step:
                    continue
                self.assertIn(step["command"], commands)
                if step["command"] == "SET_MODE":
                    self.assertIn(step["argument"], modes)
                if step["command"] == "INJECT_FAULT":
                    self.assertIn(step["argument"], faults)

    def test_release_metrics_match_v1_baseline(self) -> None:
        verification = self.model["verification"]
        self.assertEqual(verification["nativeCTest"], "20/20")
        self.assertEqual(verification["pythonTests"], "35/35")
        self.assertEqual(verification["scenarios"], "8/8")
        self.assertEqual(verification["fdir"], "10/10")
        self.assertEqual(verification["monteCarlo"], "25/25")
        self.assertEqual(verification["permissionEval"], "129/129")
        self.assertEqual(verification["requirementFailures"], 0)
        self.assertEqual(verification["traceabilityProblems"], 0)

    def test_site_assets_and_required_controls_exist(self) -> None:
        html = INDEX_PATH.read_text(encoding="utf-8")
        javascript = APP_PATH.read_text(encoding="utf-8")
        css = STYLE_PATH.read_text(encoding="utf-8")
        for element_id in (
            "commandSelect",
            "argumentSelect",
            "sendButton",
            "runScenarioButton",
            "telemetryChart",
            "eventLog",
            "evidenceGrid",
        ):
            self.assertIn(f'id="{element_id}"', html)
            self.assertIn(f"$('{element_id}')", javascript)
        self.assertIn("@media", css)
        self.assertIn("prefers-reduced-motion", css)
        self.assertTrue((REPO_ROOT / "site" / "assets" / "astra_os_architecture.svg").is_file())
        self.assertTrue((REPO_ROOT / "site" / ".nojekyll").is_file())


if __name__ == "__main__":
    unittest.main()
