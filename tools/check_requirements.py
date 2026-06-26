#!/usr/bin/env python3
"""Check AstraSim-FSW requirement traceability against YAML scenario evidence."""

from __future__ import annotations

import argparse
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import yaml


REPO_ROOT = Path(__file__).resolve().parents[1]
REQUIREMENTS_DOC = REPO_ROOT / "docs" / "requirements.md"
SCENARIOS_DIR = REPO_ROOT / "scenarios"
REPORTS_DIR = REPO_ROOT / "reports"


@dataclass
class Requirement:
    req_id: str
    text: str
    method: str
    evidence: str
    status: str


@dataclass
class ScenarioEvidence:
    name: str
    path: Path
    requirements: list[str]
    report_path: Path
    passed: bool


def parse_requirements_doc(path: Path) -> dict[str, Requirement]:
    requirements: dict[str, Requirement] = {}
    req_pattern = re.compile(r"^(FSW|VER)-REQ-\d{3}$")

    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.startswith("|"):
            continue

        columns = [column.strip() for column in line.strip().strip("|").split("|")]

        if len(columns) != 5:
            continue

        req_id, text, method, evidence, status = columns

        if not req_pattern.match(req_id):
            continue

        requirements[req_id] = Requirement(
            req_id=req_id,
            text=text,
            method=method,
            evidence=evidence,
            status=status,
        )

    return requirements


def scenario_report_path(scenario_name: str) -> Path:
    return REPORTS_DIR / f"scenario_{scenario_name}_output.txt"


def report_passed(report_path: Path) -> bool:
    if not report_path.exists():
        return False

    text = report_path.read_text(encoding="utf-8", errors="replace")
    return "Result: PASS" in text or "Scenario passed." in text


def load_scenarios(path: Path) -> list[ScenarioEvidence]:
    scenarios: list[ScenarioEvidence] = []

    for scenario_path in sorted(path.glob("*.yaml")):
        data: dict[str, Any] = yaml.safe_load(scenario_path.read_text(encoding="utf-8"))
        name = data.get("name", scenario_path.stem)
        requirements = data.get("requirements", [])

        if not isinstance(requirements, list):
            raise ValueError(f"{scenario_path}: requirements must be a list")

        report_path = scenario_report_path(name)

        scenarios.append(
            ScenarioEvidence(
                name=name,
                path=scenario_path,
                requirements=[str(req) for req in requirements],
                report_path=report_path,
                passed=report_passed(report_path),
            )
        )

    return scenarios


def write_report(
    requirements: dict[str, Requirement],
    scenarios: list[ScenarioEvidence],
    output_path: Path,
) -> tuple[int, int, int, int]:
    req_to_scenarios: dict[str, list[ScenarioEvidence]] = {
        req_id: [] for req_id in requirements
    }
    unknown_refs: set[str] = set()

    for scenario in scenarios:
        for req_id in scenario.requirements:
            if req_id in req_to_scenarios:
                req_to_scenarios[req_id].append(scenario)
            else:
                unknown_refs.add(req_id)

    lines: list[str] = []
    lines.append("# Requirement Check Report")
    lines.append("")
    lines.append("## Summary")
    lines.append("")

    passed = 0
    manual = 0
    planned = 0
    failed = 0

    status_rows: list[tuple[str, str, str, str]] = []

    for req_id in sorted(requirements):
        req = requirements[req_id]
        linked = req_to_scenarios.get(req_id, [])
        linked_passed = [scenario for scenario in linked if scenario.passed]
        linked_failed = [scenario for scenario in linked if not scenario.passed]

        if req.status == "Planned":
            result = "PLANNED"
            planned += 1
        elif linked_failed:
            result = "FAIL"
            failed += 1
        elif linked_passed:
            result = "PASS"
            passed += 1
        else:
            result = "MANUAL"
            manual += 1

        evidence_names = ", ".join(s.name for s in linked) if linked else req.evidence
        status_rows.append((req_id, result, req.status, evidence_names))

    lines.append(f"- PASS: {passed}")
    lines.append(f"- MANUAL: {manual}")
    lines.append(f"- PLANNED: {planned}")
    lines.append(f"- FAIL: {failed}")
    lines.append(f"- UNKNOWN_REFERENCES: {len(unknown_refs)}")
    lines.append("")

    lines.append("## Requirement Results")
    lines.append("")
    lines.append("| Requirement | Result | Doc Status | Evidence |")
    lines.append("|---|---|---|---|")

    for req_id, result, doc_status, evidence in status_rows:
        lines.append(f"| {req_id} | {result} | {doc_status} | {evidence} |")

    lines.append("")
    lines.append("## Scenario Evidence")
    lines.append("")
    lines.append("| Scenario | Result | Requirements | Report |")
    lines.append("|---|---|---|---|")

    for scenario in scenarios:
        result = "PASS" if scenario.passed else "FAIL"
        reqs = ", ".join(scenario.requirements) if scenario.requirements else "NONE"
        report = scenario.report_path.relative_to(REPO_ROOT)
        lines.append(f"| {scenario.name} | {result} | {reqs} | `{report}` |")

    if unknown_refs:
        lines.append("")
        lines.append("## Unknown Requirement References")
        lines.append("")
        for req_id in sorted(unknown_refs):
            lines.append(f"- {req_id}")

    output_path.parent.mkdir(exist_ok=True)
    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")

    return passed, manual, planned, failed + len(unknown_refs)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--output",
        default="reports/requirement_check_report.md",
        help="Output Markdown report path.",
    )
    args = parser.parse_args()

    requirements = parse_requirements_doc(REQUIREMENTS_DOC)
    scenarios = load_scenarios(SCENARIOS_DIR)

    output_path = REPO_ROOT / args.output
    passed, manual, planned, problems = write_report(requirements, scenarios, output_path)

    print("Requirement check complete.")
    print(f"PASS={passed}")
    print(f"MANUAL={manual}")
    print(f"PLANNED={planned}")
    print(f"PROBLEMS={problems}")
    print(f"Report written to {output_path.relative_to(REPO_ROOT)}")

    return 0 if problems == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
