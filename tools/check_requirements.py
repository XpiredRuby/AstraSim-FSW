#!/usr/bin/env python3
"""Check ASTRA-OS requirement traceability and reviewed interface baselines."""

from __future__ import annotations

import argparse
import csv
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import yaml

from traceability_baseline import (
    CONTROLLED_INTERFACE_PATHS,
    read_traceability_payload,
    requirement_fingerprint,
    sha256_file,
)

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REQUIREMENTS_DOC = REPO_ROOT / "docs" / "REQUIREMENTS.md"
DEFAULT_MATRIX_DOC = REPO_ROOT / "docs" / "VERIFICATION_MATRIX.csv"
DEFAULT_SCENARIOS_DIR = REPO_ROOT / "scenarios"
DEFAULT_TRACEABILITY_BASELINE = REPO_ROOT / "config" / "traceability_baseline.json"
DEFAULT_CMAKE = REPO_ROOT / "CMakeLists.txt"
REPORTS_DIR = REPO_ROOT / "reports"
REQUIREMENT_ID_PATTERN = re.compile(r"^(FSW|VER)-REQ-\d{3}$")
REGISTERED_TEST_PATTERN = re.compile(
    r"add_test\s*\(\s*NAME\s+([A-Za-z0-9_.+-]+)",
    re.MULTILINE,
)


@dataclass(frozen=True)
class Requirement:
    req_id: str
    text: str
    method: str
    evidence: str
    status: str


@dataclass(frozen=True)
class MatrixEntry:
    req_id: str
    component: str
    verification_case: str
    evidence: str
    status: str
    notes: str


@dataclass(frozen=True)
class ScenarioEvidence:
    name: str
    path: Path
    requirements: list[str]
    report_path: Path
    passed: bool


def parse_requirements_doc(path: Path) -> dict[str, Requirement]:
    if not path.exists():
        raise ValueError(f"requirements document does not exist: {path}")

    requirements: dict[str, Requirement] = {}

    for line_number, line in enumerate(
        path.read_text(encoding="utf-8").splitlines(),
        start=1,
    ):
        if not line.startswith("|"):
            continue

        columns = [column.strip() for column in line.strip().strip("|").split("|")]
        if len(columns) != 5:
            continue

        req_id, text, method, evidence, status = columns
        if not REQUIREMENT_ID_PATTERN.match(req_id):
            continue

        if req_id in requirements:
            raise ValueError(f"{path}:{line_number}: duplicate requirement ID {req_id}")

        if not text or not method or not status:
            raise ValueError(f"{path}:{line_number}: incomplete requirement row {req_id}")

        requirements[req_id] = Requirement(
            req_id=req_id,
            text=text,
            method=method,
            evidence=evidence,
            status=status,
        )

    if not requirements:
        raise ValueError(f"no requirements were parsed from {path}")

    return requirements


def parse_verification_matrix(path: Path) -> dict[str, MatrixEntry]:
    if not path.exists():
        raise ValueError(f"verification matrix does not exist: {path}")

    required_columns = {
        "requirement_id",
        "component",
        "verification_case",
        "evidence",
        "status",
        "notes",
    }
    entries: dict[str, MatrixEntry] = {}

    with path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        fieldnames = set(reader.fieldnames or [])
        missing_columns = sorted(required_columns - fieldnames)
        if missing_columns:
            raise ValueError(
                f"{path}: missing required columns: {', '.join(missing_columns)}"
            )

        for line_number, row in enumerate(reader, start=2):
            req_id = (row.get("requirement_id") or "").strip()
            if not REQUIREMENT_ID_PATTERN.match(req_id):
                raise ValueError(f"{path}:{line_number}: invalid requirement ID {req_id!r}")
            if req_id in entries:
                raise ValueError(f"{path}:{line_number}: duplicate matrix entry {req_id}")

            entry = MatrixEntry(
                req_id=req_id,
                component=(row.get("component") or "").strip(),
                verification_case=(row.get("verification_case") or "").strip(),
                evidence=(row.get("evidence") or "").strip(),
                status=(row.get("status") or "").strip(),
                notes=(row.get("notes") or "").strip(),
            )

            if not entry.component:
                raise ValueError(f"{path}:{line_number}: {req_id} has no component allocation")
            if entry.status != "Planned" and not entry.verification_case:
                raise ValueError(
                    f"{path}:{line_number}: non-planned {req_id} has no verification case"
                )

            entries[req_id] = entry

    return entries


def parse_registered_ctests(path: Path) -> set[str]:
    if not path.exists():
        raise ValueError(f"CMake file does not exist: {path}")
    text = path.read_text(encoding="utf-8")
    return set(REGISTERED_TEST_PATTERN.findall(text))


def matrix_verification_tokens(matrix: dict[str, MatrixEntry]) -> set[str]:
    tokens: set[str] = set()
    for entry in matrix.values():
        for token in entry.verification_case.split(";"):
            normalized = token.strip()
            if normalized:
                tokens.add(normalized)
    return tokens


def scenario_report_path(scenario_name: str) -> Path:
    return REPORTS_DIR / f"scenario_{scenario_name}_output.txt"


def report_passed(report_path: Path) -> bool:
    if not report_path.exists():
        return False

    text = report_path.read_text(encoding="utf-8", errors="replace")
    return "Result: PASS" in text or "Scenario passed." in text


def evidence_report_passed(evidence: str) -> bool:
    report_paths = re.findall(r"`(reports/[^`]+)`", evidence)

    for report_path_text in report_paths:
        report_path = REPO_ROOT / report_path_text
        if not report_path.is_file():
            continue
        text = report_path.read_text(encoding="utf-8", errors="replace")
        if (
            "Result: PASS" in text
            or "100% tests passed" in text
            or "overall_status\": \"passed" in text
        ):
            return True

    return False


def load_scenarios(path: Path) -> list[ScenarioEvidence]:
    if not path.exists():
        raise ValueError(f"scenario directory does not exist: {path}")

    scenarios: list[ScenarioEvidence] = []

    for scenario_path in sorted(path.glob("*.yaml")):
        data: dict[str, Any] = yaml.safe_load(
            scenario_path.read_text(encoding="utf-8")
        )
        if not isinstance(data, dict):
            raise ValueError(f"{scenario_path}: scenario root must be a mapping")

        name = str(data.get("name", scenario_path.stem))
        requirements = data.get("requirements", [])
        if not isinstance(requirements, list):
            raise ValueError(f"{scenario_path}: requirements must be a list")

        report_path = scenario_report_path(name)
        scenarios.append(
            ScenarioEvidence(
                name=name,
                path=scenario_path,
                requirements=[str(requirement) for requirement in requirements],
                report_path=report_path,
                passed=report_passed(report_path),
            )
        )

    return scenarios


def matrix_consistency_problems(
    requirements: dict[str, Requirement],
    matrix: dict[str, MatrixEntry],
) -> list[str]:
    problems: list[str] = []

    for req_id in sorted(set(requirements) - set(matrix)):
        problems.append(f"Requirement missing from matrix: {req_id}")

    for req_id in sorted(set(matrix) - set(requirements)):
        problems.append(f"Matrix references unknown requirement: {req_id}")

    for req_id in sorted(set(requirements) & set(matrix)):
        requirement = requirements[req_id]
        entry = matrix[req_id]
        if requirement.status != entry.status:
            problems.append(
                f"Status mismatch for {req_id}: "
                f"requirements={requirement.status}, matrix={entry.status}"
            )

    return problems


def reverse_test_allocation_problems(
    registered_tests: set[str],
    matrix: dict[str, MatrixEntry],
) -> list[str]:
    allocated = matrix_verification_tokens(matrix)
    return [
        f"Registered CTest has no requirement allocation: {test_name}"
        for test_name in sorted(registered_tests - allocated)
    ]


def reviewed_baseline_problems(
    requirements: dict[str, Requirement],
    baseline_path: Path,
) -> list[str]:
    if not baseline_path.exists():
        return [
            "Traceability baseline is missing; run "
            "python3 tools/update_traceability_baseline.py after review"
        ]

    payload = read_traceability_payload(baseline_path)
    baseline_requirements = payload["requirements"]
    baseline_interfaces = payload["controlled_interfaces"]
    assert isinstance(baseline_requirements, dict)
    assert isinstance(baseline_interfaces, dict)

    problems: list[str] = []
    current_ids = set(requirements)
    baseline_ids = set(str(key) for key in baseline_requirements)

    for req_id in sorted(current_ids - baseline_ids):
        problems.append(f"Requirement added since review baseline: {req_id}")
    for req_id in sorted(baseline_ids - current_ids):
        problems.append(f"Requirement removed since review baseline: {req_id}")
    for req_id in sorted(current_ids & baseline_ids):
        expected = str(baseline_requirements[req_id])
        actual = requirement_fingerprint(requirements[req_id])
        if actual != expected:
            problems.append(f"Requirement changed since review baseline: {req_id}")

    expected_interface_paths = set(CONTROLLED_INTERFACE_PATHS)
    baseline_interface_paths = set(str(key) for key in baseline_interfaces)
    for relative_path in sorted(expected_interface_paths - baseline_interface_paths):
        problems.append(
            f"Controlled interface missing from review baseline: {relative_path}"
        )
    for relative_path in sorted(baseline_interface_paths - expected_interface_paths):
        problems.append(
            f"Unexpected controlled interface in review baseline: {relative_path}"
        )
    for relative_path in sorted(expected_interface_paths & baseline_interface_paths):
        path = REPO_ROOT / relative_path
        if not path.is_file():
            problems.append(f"Controlled interface is missing: {relative_path}")
            continue
        expected = str(baseline_interfaces[relative_path])
        actual = sha256_file(path)
        if actual != expected:
            problems.append(
                f"Controlled interface changed since review baseline: {relative_path}"
            )

    return problems


def write_report(
    requirements: dict[str, Requirement],
    matrix: dict[str, MatrixEntry],
    scenarios: list[ScenarioEvidence],
    output_path: Path,
    registered_tests: set[str],
    baseline_path: Path,
) -> tuple[dict[str, int], list[str]]:
    req_to_scenarios: dict[str, list[ScenarioEvidence]] = {
        req_id: [] for req_id in requirements
    }
    problems = matrix_consistency_problems(requirements, matrix)
    problems.extend(reverse_test_allocation_problems(registered_tests, matrix))
    problems.extend(reviewed_baseline_problems(requirements, baseline_path))

    for scenario in scenarios:
        for req_id in scenario.requirements:
            if req_id in req_to_scenarios:
                req_to_scenarios[req_id].append(scenario)
            else:
                problems.append(
                    f"Scenario {scenario.path.relative_to(REPO_ROOT)} "
                    f"references unknown requirement {req_id}"
                )

    counts = {
        "PASS": 0,
        "MANUAL": 0,
        "PLANNED": 0,
        "HISTORICAL": 0,
        "FAIL": 0,
    }
    status_rows: list[tuple[str, str, str, str, str]] = []

    for req_id in sorted(requirements):
        requirement = requirements[req_id]
        entry = matrix.get(req_id)
        linked = req_to_scenarios.get(req_id, [])
        linked_passed = [scenario for scenario in linked if scenario.passed]
        linked_failed = [scenario for scenario in linked if not scenario.passed]

        if requirement.status == "Planned":
            result = "PLANNED"
        elif requirement.status == "Historical":
            result = "HISTORICAL"
        elif linked_failed:
            result = "FAIL"
        elif linked_passed or evidence_report_passed(requirement.evidence):
            result = "PASS"
        else:
            result = "MANUAL"

        counts[result] += 1
        evidence_names = ", ".join(scenario.name for scenario in linked)
        if not evidence_names and entry is not None:
            evidence_names = entry.verification_case
        if not evidence_names:
            evidence_names = requirement.evidence

        component = entry.component if entry is not None else "MISSING"
        status_rows.append(
            (req_id, result, requirement.status, component, evidence_names)
        )

    lines: list[str] = [
        "# Requirement Check Report",
        "",
        "## Summary",
        "",
    ]

    for name in ("PASS", "MANUAL", "PLANNED", "HISTORICAL", "FAIL"):
        lines.append(f"- {name}: {counts[name]}")
    lines.append(f"- REGISTERED_CTESTS: {len(registered_tests)}")
    lines.append(f"- TRACEABILITY_PROBLEMS: {len(problems)}")
    lines.append("")
    lines.append(
        "A MANUAL result means the requirement has a matrix allocation but was "
        "not automatically dispositioned by scenario or committed report logic. "
        "It is not equivalent to PASS."
    )
    lines.append("")
    lines.extend(
        [
            "## Requirement Results",
            "",
            "| Requirement | Result | Doc Status | Component | Verification Case or Evidence |",
            "|---|---|---|---|---|",
        ]
    )

    for req_id, result, doc_status, component, evidence in status_rows:
        lines.append(
            f"| {req_id} | {result} | {doc_status} | {component} | {evidence} |"
        )

    lines.extend(
        [
            "",
            "## Scenario Evidence",
            "",
            "| Scenario | Result | Requirements | Report |",
            "|---|---|---|---|",
        ]
    )

    for scenario in scenarios:
        result = "PASS" if scenario.passed else "FAIL"
        reqs = ", ".join(scenario.requirements) if scenario.requirements else "NONE"
        report = scenario.report_path.relative_to(REPO_ROOT)
        lines.append(f"| {scenario.name} | {result} | {reqs} | `{report}` |")

    lines.extend(
        [
            "",
            "## Reverse CTest Allocation",
            "",
            f"Registered CTests: `{len(registered_tests)}`",
            "",
            "Every registered CTest name must appear in at least one verification-matrix case list.",
        ]
    )

    if problems:
        lines.extend(["", "## Traceability Problems", ""])
        for problem in problems:
            lines.append(f"- {problem}")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return counts, problems


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--requirements", type=Path, default=DEFAULT_REQUIREMENTS_DOC)
    parser.add_argument("--matrix", type=Path, default=DEFAULT_MATRIX_DOC)
    parser.add_argument("--scenarios", type=Path, default=DEFAULT_SCENARIOS_DIR)
    parser.add_argument("--cmake", type=Path, default=DEFAULT_CMAKE)
    parser.add_argument(
        "--traceability-baseline",
        type=Path,
        default=DEFAULT_TRACEABILITY_BASELINE,
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("reports/requirement_check_report.md"),
        help="Output Markdown report path.",
    )
    return parser.parse_args()


def resolve_repo_path(path: Path) -> Path:
    return path if path.is_absolute() else REPO_ROOT / path


def main() -> int:
    args = parse_args()

    try:
        requirements = parse_requirements_doc(resolve_repo_path(args.requirements))
        matrix = parse_verification_matrix(resolve_repo_path(args.matrix))
        scenarios = load_scenarios(resolve_repo_path(args.scenarios))
        registered_tests = parse_registered_ctests(resolve_repo_path(args.cmake))
        baseline_path = resolve_repo_path(args.traceability_baseline)
        output_path = resolve_repo_path(args.output)
        counts, problems = write_report(
            requirements,
            matrix,
            scenarios,
            output_path,
            registered_tests,
            baseline_path,
        )
    except (OSError, ValueError, yaml.YAMLError) as error:
        print(f"ERROR: {error}")
        return 1

    print("Requirement check complete.")
    for name in ("PASS", "MANUAL", "PLANNED", "HISTORICAL", "FAIL"):
        print(f"{name}={counts[name]}")
    print(f"REGISTERED_CTESTS={len(registered_tests)}")
    print(f"TRACEABILITY_PROBLEMS={len(problems)}")
    print(f"Report written to {output_path.relative_to(REPO_ROOT)}")

    return 0 if counts["FAIL"] == 0 and not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
