#!/usr/bin/env python3
"""Generate per-production-module coverage summaries from an LCOV tracefile."""

from __future__ import annotations

import argparse
import csv
import json
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

REPO_ROOT = Path(__file__).resolve().parents[1]


@dataclass
class CoverageRecord:
    path: str
    lines_found: int = 0
    lines_hit: int = 0
    functions_found: int = 0
    functions_hit: int = 0
    branches_found: int = 0
    branches_hit: int = 0

    @staticmethod
    def percent(hit: int, found: int) -> float | None:
        if found == 0:
            return None
        return round(100.0 * hit / found, 2)

    def to_dict(self) -> dict[str, object]:
        payload = asdict(self)
        payload["line_percent"] = self.percent(self.lines_hit, self.lines_found)
        payload["function_percent"] = self.percent(self.functions_hit, self.functions_found)
        payload["branch_percent"] = self.percent(self.branches_hit, self.branches_found)
        return payload


def normalize_source_path(source: str) -> str:
    path = Path(source)
    try:
        return str(path.resolve().relative_to(REPO_ROOT.resolve()))
    except ValueError:
        return str(path)


def parse_lcov(lines: Iterable[str]) -> list[CoverageRecord]:
    records: list[CoverageRecord] = []
    current: CoverageRecord | None = None

    for raw_line in lines:
        line = raw_line.strip()
        if not line:
            continue

        if line.startswith("SF:"):
            if current is not None:
                raise ValueError("encountered a new source file before end_of_record")
            current = CoverageRecord(path=normalize_source_path(line[3:]))
            continue

        if line == "end_of_record":
            if current is None:
                raise ValueError("end_of_record without source file")
            records.append(current)
            current = None
            continue

        if current is None:
            continue

        key, separator, value = line.partition(":")
        if not separator:
            continue

        if key == "LF":
            current.lines_found = int(value)
        elif key == "LH":
            current.lines_hit = int(value)
        elif key == "FNF":
            current.functions_found = int(value)
        elif key == "FNH":
            current.functions_hit = int(value)
        elif key == "BRF":
            current.branches_found = int(value)
        elif key == "BRH":
            current.branches_hit = int(value)

    if current is not None:
        raise ValueError("LCOV tracefile ended before end_of_record")

    return sorted(records, key=lambda record: record.path)


def total_record(records: list[CoverageRecord]) -> CoverageRecord:
    total = CoverageRecord(path="TOTAL")
    for record in records:
        total.lines_found += record.lines_found
        total.lines_hit += record.lines_hit
        total.functions_found += record.functions_found
        total.functions_hit += record.functions_hit
        total.branches_found += record.branches_found
        total.branches_hit += record.branches_hit
    return total


def percent_text(value: float | None) -> str:
    return "N/A" if value is None else f"{value:.2f}%"


def write_csv(path: Path, records: list[CoverageRecord]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = [
        "path",
        "lines_found",
        "lines_hit",
        "line_percent",
        "functions_found",
        "functions_hit",
        "function_percent",
        "branches_found",
        "branches_hit",
        "branch_percent",
    ]
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for record in [*records, total_record(records)]:
            writer.writerow(record.to_dict())


def write_json(path: Path, records: list[CoverageRecord]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "schema": "astra-os.module-coverage.v1",
        "claim_boundary": (
            "Observed structural coverage for this exact run. No certification, "
            "adequacy, or reliability conclusion is implied."
        ),
        "module_count": len(records),
        "modules": [record.to_dict() for record in records],
        "total": total_record(records).to_dict(),
    }
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def write_markdown(path: Path, records: list[CoverageRecord]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "# ASTRA-OS Per-Module Structural Coverage",
        "",
        "Observed coverage for the exact instrumented run that produced the linked LCOV tracefile. No adequacy, certification, reliability, or safety conclusion is implied by these percentages.",
        "",
        "| Production module | Lines | Functions | Branches |",
        "|---|---:|---:|---:|",
    ]

    for record in records:
        values = record.to_dict()
        lines.append(
            "| `{path}` | {lines_hit}/{lines_found} ({line_percent}) | "
            "{functions_hit}/{functions_found} ({function_percent}) | "
            "{branches_hit}/{branches_found} ({branch_percent}) |".format(
                path=record.path,
                lines_hit=record.lines_hit,
                lines_found=record.lines_found,
                line_percent=percent_text(values["line_percent"]),
                functions_hit=record.functions_hit,
                functions_found=record.functions_found,
                function_percent=percent_text(values["function_percent"]),
                branches_hit=record.branches_hit,
                branches_found=record.branches_found,
                branch_percent=percent_text(values["branch_percent"]),
            )
        )

    total = total_record(records)
    values = total.to_dict()
    lines.extend(
        [
            "| **TOTAL** | {hit}/{found} ({percent}) | {f_hit}/{f_found} ({f_percent}) | {b_hit}/{b_found} ({b_percent}) |".format(
                hit=total.lines_hit,
                found=total.lines_found,
                percent=percent_text(values["line_percent"]),
                f_hit=total.functions_hit,
                f_found=total.functions_found,
                f_percent=percent_text(values["function_percent"]),
                b_hit=total.branches_hit,
                b_found=total.branches_found,
                b_percent=percent_text(values["branch_percent"]),
            ),
            "",
            "## Exclusions",
            "",
            "The CI collection step removes system headers and `fsw/tests/*`. It does not exclude production files merely because they have low coverage. Any future exclusion must be documented with a technical justification.",
        ]
    )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("tracefile", type=Path)
    parser.add_argument("--csv", dest="csv_path", type=Path, required=True)
    parser.add_argument("--json", dest="json_path", type=Path, required=True)
    parser.add_argument("--markdown", dest="markdown_path", type=Path, required=True)
    return parser.parse_args()


def resolve(path: Path) -> Path:
    return path if path.is_absolute() else REPO_ROOT / path


def main() -> int:
    args = parse_args()
    tracefile = resolve(args.tracefile)

    try:
        records = parse_lcov(tracefile.read_text(encoding="utf-8").splitlines())
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}")
        return 1

    if not records:
        print("ERROR: no source records found in LCOV tracefile")
        return 1

    write_csv(resolve(args.csv_path), records)
    write_json(resolve(args.json_path), records)
    write_markdown(resolve(args.markdown_path), records)
    print(f"Generated per-module coverage evidence for {len(records)} production files.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
