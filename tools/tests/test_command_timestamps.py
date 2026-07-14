from __future__ import annotations

import struct
import unittest

from tools.run_scenario import resolve_step_timestamp
from tools.send_command import build_packet


class CommandTimestampTests(unittest.TestCase):
    def test_build_packet_uses_explicit_timestamp(self) -> None:
        packet = build_packet(7, "NOOP", 0, timestamp_ms=123456789)
        fields = struct.unpack("<IHIQHIH", packet)
        self.assertEqual(fields[3], 123456789)

    def test_relative_scenario_timestamp_uses_supplied_clock(self) -> None:
        timestamp = resolve_step_timestamp(
            {"timestamp_offset_ms": -6001},
            now_ms=10000,
        )
        self.assertEqual(timestamp, 3999)

    def test_exact_scenario_timestamp_is_preserved(self) -> None:
        self.assertEqual(
            resolve_step_timestamp({"timestamp_ms": 42}, now_ms=10000),
            42,
        )

    def test_unspecified_timestamp_uses_sender_default(self) -> None:
        self.assertIsNone(resolve_step_timestamp({}, now_ms=10000))

    def test_exact_and_relative_timestamp_are_mutually_exclusive(self) -> None:
        with self.assertRaises(ValueError):
            resolve_step_timestamp(
                {"timestamp_ms": 1, "timestamp_offset_ms": 2},
                now_ms=10000,
            )


if __name__ == "__main__":
    unittest.main()
