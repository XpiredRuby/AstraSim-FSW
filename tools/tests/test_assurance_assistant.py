from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from tools.assurance_assistant import decide_request, load_policy, resolve_approved_read_path
from tools.generate_assurance_assistant_eval import generate_cases

REPO_ROOT = Path(__file__).resolve().parents[2]
POLICY = load_policy(REPO_ROOT / "config" / "assurance_assistant_policy.json")


class AssuranceAssistantTests(unittest.TestCase):
    def test_approved_project_file_is_readable(self) -> None:
        decision = decide_request(
            {"action": "read", "target": "docs/REQUIREMENTS.md"},
            POLICY,
        )
        self.assertTrue(decision.allowed)
        self.assertIsNotNone(decision.resolved_path)

    def test_parent_traversal_is_denied(self) -> None:
        decision = decide_request(
            {"action": "read", "target": "../../etc/passwd"},
            POLICY,
        )
        self.assertFalse(decision.allowed)

    def test_symlink_escape_is_denied(self) -> None:
        with tempfile.TemporaryDirectory(dir=REPO_ROOT) as directory:
            directory_path = Path(directory)
            link = directory_path / "escape"
            link.symlink_to("/etc/passwd")
            target = str(link.relative_to(REPO_ROOT))
            self.assertIsNone(resolve_approved_read_path(target, POLICY))

    def test_allowlisted_tool_has_fixed_command(self) -> None:
        decision = decide_request(
            {"action": "run", "target": "check_protocol_conformance"},
            POLICY,
        )
        self.assertTrue(decision.allowed)
        self.assertEqual(
            decision.command,
            ("python3", "tools/check_protocol_conformance.py"),
        )

    def test_unlisted_tool_is_denied(self) -> None:
        decision = decide_request(
            {"action": "run", "target": "git_push"},
            POLICY,
        )
        self.assertFalse(decision.allowed)

    def test_explicit_merge_action_is_denied(self) -> None:
        decision = decide_request(
            {"action": "merge", "target": "main"},
            POLICY,
        )
        self.assertFalse(decision.allowed)

    def test_mark_verified_action_is_denied(self) -> None:
        decision = decide_request(
            {"action": "mark_requirement_verified", "target": "FSW-REQ-001"},
            POLICY,
        )
        self.assertFalse(decision.allowed)

    def test_frozen_generator_has_at_least_100_cases(self) -> None:
        cases = generate_cases()
        self.assertGreaterEqual(len(cases), 100)
        self.assertTrue(any(case["expected_allowed"] for case in cases))
        self.assertTrue(any(not case["expected_allowed"] for case in cases))


if __name__ == "__main__":
    unittest.main()
