#!/usr/bin/env python3
"""Update the reviewed ASTRA-OS traceability fingerprint baseline."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from check_requirements import parse_requirements_doc
from traceability_baseline import build_traceability_payload

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REQUIREMENTS = REPO_ROOT / "docs" / "REQUIREMENTS.md"
DEFAULT_OUTPUT = REPO_ROOT / "config" / "traceability_baseline.json"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--requirements", type=Path, default=DEFAULT_REQUIREMENTS)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    requirements_path = (
        args.requirements
        if args.requirements.is_absolute()
        else REPO_ROOT / args.requirements
    )
    output_path = args.output if args.output.is_absolute() else REPO_ROOT / args.output

    try:
        requirements = parse_requirements_doc(requirements_path)
        payload = build_traceability_payload(requirements, REPO_ROOT)
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}")
        return 1

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    print(f"Wrote traceability baseline: {output_path.relative_to(REPO_ROOT)}")
    print(f"Requirement fingerprints: {len(payload['requirements'])}")
    print(f"Controlled interfaces: {len(payload['controlled_interfaces'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
