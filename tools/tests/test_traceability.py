from __future__ import annotations

import json
import tempfile
import unittest
from dataclasses import dataclass
from pathlib import Path

from tools.check_requirements import (
    MatrixEntry,
    matrix_verification_tokens,
    parse_registered_ctests,
    reverse_test_allocation_problems,
)
from tools.traceability_baseline import (
    TRACEABILITY_SCHEMA,
    read_traceability_payload,
    requirement_fingerprint,
)


@dataclass(frozen=True)
class FakeRequirement:
    req_id: str
    text: str
    method: str
    status: str


class TraceabilityTests(unittest.TestCase):
    def test_requirement_fingerprint_changes_with_reviewed_fields(self) -> None:
        original = FakeRequirement("FSW-REQ-001", "Initial text", "Unit test", "Verified")
        changed = FakeRequirement("FSW-REQ-001", "Changed text", "Unit test", "Verified")
        self.assertNotEqual(
            requirement_fingerprint(original),
            requirement_fingerprint(changed),
        )

    def test_requirement_fingerprint_ignores_evidence_formatting(self) -> None:
        first = FakeRequirement("FSW-REQ-001", "Text", "Unit test", "Verified")
        second = FakeRequirement("FSW-REQ-001", "Text", "Unit test", "Verified")
        self.assertEqual(
            requirement_fingerprint(first),
            requirement_fingerprint(second),
        )

    def test_registered_ctest_parser_finds_names(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            cmake = Path(directory) / "CMakeLists.txt"
            cmake.write_text(
                "add_test(NAME alpha_tests COMMAND alpha)\n"
                "add_test( NAME beta-tests COMMAND beta )\n",
                encoding="utf-8",
            )
            self.assertEqual(
                parse_registered_ctests(cmake),
                {"alpha_tests", "beta-tests"},
            )

    def test_reverse_allocation_reports_unmapped_test(self) -> None:
        matrix = {
            "FSW-REQ-001": MatrixEntry(
                req_id="FSW-REQ-001",
                component="Core",
                verification_case="alpha_tests;scenario_one",
                evidence="CTest",
                status="Verified",
                notes="",
            )
        }
        self.assertEqual(
            matrix_verification_tokens(matrix),
            {"alpha_tests", "scenario_one"},
        )
        self.assertEqual(
            reverse_test_allocation_problems(
                {"alpha_tests", "orphan_tests"},
                matrix,
            ),
            ["Registered CTest has no requirement allocation: orphan_tests"],
        )

    def test_traceability_payload_validation(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "baseline.json"
            path.write_text(
                json.dumps(
                    {
                        "schema": TRACEABILITY_SCHEMA,
                        "requirements": {"FSW-REQ-001": "abc"},
                        "controlled_interfaces": {"one": "def"},
                    }
                ),
                encoding="utf-8",
            )
            payload = read_traceability_payload(path)
            self.assertEqual(payload["schema"], TRACEABILITY_SCHEMA)

    def test_traceability_payload_rejects_wrong_schema(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "baseline.json"
            path.write_text(
                json.dumps(
                    {
                        "schema": "wrong",
                        "requirements": {},
                        "controlled_interfaces": {},
                    }
                ),
                encoding="utf-8",
            )
            with self.assertRaises(ValueError):
                read_traceability_payload(path)


if __name__ == "__main__":
    unittest.main()
