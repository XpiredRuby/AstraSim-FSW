#!/usr/bin/env python3

from __future__ import annotations

import unittest

from tools.summarize_lcov import parse_lcov, total_record


class SummarizeLcovTests(unittest.TestCase):
    def test_parses_multiple_records_and_totals(self) -> None:
        trace = [
            "TN:",
            "SF:/tmp/project/fsw/src/first.cpp",
            "FNF:2",
            "FNH:1",
            "BRF:4",
            "BRH:3",
            "LF:10",
            "LH:8",
            "end_of_record",
            "SF:/tmp/project/fsw/src/second.cpp",
            "FNF:1",
            "FNH:1",
            "LF:5",
            "LH:5",
            "end_of_record",
        ]

        records = parse_lcov(trace)
        self.assertEqual(len(records), 2)
        self.assertEqual(records[0].lines_found, 10)
        self.assertEqual(records[0].lines_hit, 8)
        self.assertEqual(records[0].to_dict()["line_percent"], 80.0)
        self.assertIsNone(records[1].to_dict()["branch_percent"])

        total = total_record(records)
        self.assertEqual(total.lines_found, 15)
        self.assertEqual(total.lines_hit, 13)
        self.assertEqual(total.functions_found, 3)
        self.assertEqual(total.functions_hit, 2)
        self.assertEqual(total.branches_found, 4)
        self.assertEqual(total.branches_hit, 3)

    def test_requires_end_of_record(self) -> None:
        with self.assertRaisesRegex(ValueError, "ended before end_of_record"):
            parse_lcov(["SF:/tmp/file.cpp", "LF:1", "LH:1"])

    def test_rejects_nested_source_record(self) -> None:
        with self.assertRaisesRegex(ValueError, "new source file"):
            parse_lcov(
                [
                    "SF:/tmp/first.cpp",
                    "SF:/tmp/second.cpp",
                    "end_of_record",
                ]
            )

    def test_rejects_orphan_end_record(self) -> None:
        with self.assertRaisesRegex(ValueError, "without source file"):
            parse_lcov(["end_of_record"])


if __name__ == "__main__":
    unittest.main()
