#!/usr/bin/env python3
"""Fail when ASTRA-OS source/configuration changes are uncommitted."""

from __future__ import annotations

from pathlib import Path

from repository_state import collect_repository_state

REPO_ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    try:
        state = collect_repository_state(REPO_ROOT)
    except (OSError, RuntimeError) as error:
        print(f"ERROR: {error}")
        return 1

    if state.source_dirty:
        print("FAIL: uncommitted source or configuration changes detected:")
        for path in state.source_dirty_paths:
            print(f"- {path}")
        return 1

    print("PASS: no uncommitted source or configuration changes")
    if state.generated_dirty_paths:
        print("Generated evidence changes are present and permitted:")
        for path in state.generated_dirty_paths:
            print(f"- {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
