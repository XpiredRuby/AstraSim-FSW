#!/usr/bin/env python3
"""Run the frozen ASTRA-OS assurance-assistant permission evaluation."""

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path
from typing import Any

from assurance_assistant import decide_request, load_policy

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_POLICY = REPO_ROOT / "config" / "assurance_assistant_policy.json"
DEFAULT_EVAL = REPO_ROOT / "config" / "assurance_assistant_eval.json"
DEFAULT_JSON = REPO_ROOT / "reports" / "assurance_assistant_eval.json"
DEFAULT_MARKDOWN = REPO_ROOT / "reports" / "assurance_assistant_eval.md"


def load_evaluation(path: Path) -> dict[str, Any]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise ValueError(f"unable to read evaluation set {path}: {error}") from error

    if payload.get("schema") != "astra-os.assurance-assistant-eval.v1":
        raise ValueError(f"unexpected evaluation schema in {path}")
    cases = payload.get("cases")
    if not isinstance(cases, list):
        raise ValueError("evaluation cases must be a list")
    if len(cases) < 100:
        raise ValueError("evaluation set must contain at least 100 frozen cases")
    if payload.get("case_count") != len(cases):
        raise ValueError("evaluation case_count does not match cases")
    return payload


def evaluate(
    policy: dict[str, Any],
    payload: dict[str, Any],
) -> tuple[list[dict[str, object]], Counter[str]]:
    results: list[dict[str, object]] = []
    counts: Counter[str] = Counter()

    for raw_case in payload["cases"]:
        if not isinstance(raw_case, dict):
            raise ValueError("each evaluation case must be an object")
        case_id = str(raw_case.get("id", ""))
        category = str(raw_case.get("category", "uncategorized"))
        request = raw_case.get("request")
        expected = raw_case.get("expected_allowed")
        if not case_id or not isinstance(request, dict) or not isinstance(expected, bool):
            raise ValueError(f"invalid evaluation case: {raw_case}")

        decision = decide_request(request, policy)
        passed = decision.allowed == expected
        counts["passed" if passed else "failed"] += 1
        counts[f"category:{category}:total"] += 1
        if passed:
            counts[f"category:{category}:passed"] += 1

        results.append(
            {
                "id": case_id,
                "category": category,
                "request": request,
                "expected_allowed": expected,
                "actual_allowed": decision.allowed,
                "passed": passed,
                "reason": decision.reason,
            }
        )

    return results, counts


def write_json(
    path: Path,
    source: Path,
    results: list[dict[str, object]],
    counts: Counter[str],
) -> None:
    total = len(results)
    passed = counts["passed"]
    failed = counts["failed"]
    payload = {
        "schema": "astra-os.assurance-assistant-eval-result.v1",
        "claim_boundary": (
            "Observed results for the committed deterministic permission cases only. "
            "No general AI safety or security certification is implied."
        ),
        "evaluation_source": str(source.relative_to(REPO_ROOT)),
        "total": total,
        "passed": passed,
        "failed": failed,
        "pass_percent": round(100.0 * passed / total, 2) if total else 0.0,
        "result": "PASS" if failed == 0 else "FAIL",
        "results": results,
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def write_markdown(
    path: Path,
    source: Path,
    results: list[dict[str, object]],
    counts: Counter[str],
) -> None:
    total = len(results)
    passed = counts["passed"]
    failed = counts["failed"]
    categories = sorted(
        key.split(":", 2)[1]
        for key in counts
        if key.startswith("category:") and key.endswith(":total")
    )

    lines = [
        "# ASTRA-OS Assurance Assistant Frozen Evaluation",
        "",
        f"Result: {'PASS' if failed == 0 else 'FAIL'}",
        "",
        f"Evaluation source: `{source.relative_to(REPO_ROOT)}`",
        f"Cases: `{total}`",
        f"Passed: `{passed}`",
        f"Failed: `{failed}`",
        f"Pass rate: `{(100.0 * passed / total if total else 0.0):.2f}%`",
        "",
        "This evaluates only the deterministic read/tool authorization boundary. It does not establish general AI safety, cybersecurity certification, or protection outside the implemented interface.",
        "",
        "## Category results",
        "",
        "| Category | Passed | Total |",
        "|---|---:|---:|",
    ]

    for category in categories:
        category_passed = counts[f"category:{category}:passed"]
        category_total = counts[f"category:{category}:total"]
        lines.append(f"| `{category}` | {category_passed} | {category_total} |")

    failures = [result for result in results if not bool(result["passed"])]
    if failures:
        lines.extend(["", "## Failures", ""])
        for result in failures:
            lines.append(
                f"- `{result['id']}` expected allowed={result['expected_allowed']} "
                f"but observed allowed={result['actual_allowed']}: {result['reason']}"
            )

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--policy", type=Path, default=DEFAULT_POLICY)
    parser.add_argument("--evaluation", type=Path, default=DEFAULT_EVAL)
    parser.add_argument("--json", dest="json_output", type=Path, default=DEFAULT_JSON)
    parser.add_argument("--markdown", dest="markdown_output", type=Path, default=DEFAULT_MARKDOWN)
    args = parser.parse_args()

    policy_path = args.policy if args.policy.is_absolute() else REPO_ROOT / args.policy
    evaluation_path = (
        args.evaluation if args.evaluation.is_absolute() else REPO_ROOT / args.evaluation
    )
    json_output = (
        args.json_output if args.json_output.is_absolute() else REPO_ROOT / args.json_output
    )
    markdown_output = (
        args.markdown_output
        if args.markdown_output.is_absolute()
        else REPO_ROOT / args.markdown_output
    )

    try:
        policy = load_policy(policy_path)
        payload = load_evaluation(evaluation_path)
        results, counts = evaluate(policy, payload)
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}")
        return 1

    write_json(json_output, evaluation_path, results, counts)
    write_markdown(markdown_output, evaluation_path, results, counts)

    print(f"Cases: {len(results)}")
    print(f"Passed: {counts['passed']}")
    print(f"Failed: {counts['failed']}")
    print(f"JSON report: {json_output.relative_to(REPO_ROOT)}")
    print(f"Markdown report: {markdown_output.relative_to(REPO_ROOT)}")
    return 0 if counts["failed"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
