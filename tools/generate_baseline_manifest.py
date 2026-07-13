#!/usr/bin/env python3
"""Generate a machine-readable ASTRA-OS build and verification provenance manifest."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import platform
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = REPO_ROOT / "reports" / "latest" / "baseline_manifest.json"


def run_probe(command: list[str]) -> dict[str, Any]:
    executable = shutil.which(command[0])
    if executable is None:
        return {
            "command": command,
            "available": False,
            "return_code": None,
            "output": None,
        }

    try:
        completed = subprocess.run(
            command,
            cwd=REPO_ROOT,
            check=False,
            capture_output=True,
            text=True,
            timeout=15,
        )
    except (OSError, subprocess.TimeoutExpired) as error:
        return {
            "command": command,
            "available": True,
            "return_code": None,
            "output": None,
            "error": str(error),
        }

    output = (completed.stdout or completed.stderr).strip()
    return {
        "command": command,
        "available": True,
        "return_code": completed.returncode,
        "output": output,
    }


def git_text(*arguments: str) -> str | None:
    probe = run_probe(["git", *arguments])
    if probe["return_code"] != 0 or not probe["output"]:
        return None
    return str(probe["output"]).splitlines()[0]


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def evidence_inputs() -> list[Path]:
    candidates: list[Path] = [
        REPO_ROOT / "CMakeLists.txt",
        REPO_ROOT / ".gitignore",
    ]

    glob_patterns = (
        "*.md",
        "fsw/include/astra/*.hpp",
        "fsw/src/*.cpp",
        "fsw/tests/*.cpp",
        "fsw/fuzz/*.cpp",
        "config/*.json",
        "scenarios/*.yaml",
        "ci/*.sh",
        "tools/*.py",
        ".github/workflows/*.yml",
        "docs/*.md",
        "docs/*.csv",
    )

    for pattern in glob_patterns:
        candidates.extend(REPO_ROOT.glob(pattern))

    return sorted({path for path in candidates if path.is_file()})


def file_manifest() -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for path in evidence_inputs():
        records.append(
            {
                "path": str(path.relative_to(REPO_ROOT)),
                "size_bytes": path.stat().st_size,
                "sha256": sha256_file(path),
            }
        )
    return records


def build_manifest(args: argparse.Namespace) -> dict[str, Any]:
    git_status_probe = run_probe(["git", "status", "--porcelain"])
    git_status_output = git_status_probe.get("output") or ""
    cxx = os.environ.get("CXX", "c++")

    return {
        "schema": "astra-os.provenance-manifest.v1",
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "verification": {
            "status": args.verification_status,
            "commands": args.test_command,
            "note": args.note,
        },
        "repository": {
            "root": str(REPO_ROOT),
            "commit": git_text("rev-parse", "HEAD"),
            "branch": git_text("rev-parse", "--abbrev-ref", "HEAD"),
            "describe": git_text("describe", "--always", "--dirty", "--tags"),
            "dirty": bool(str(git_status_output).strip()),
            "remote": git_text("remote", "get-url", "origin"),
        },
        "host": {
            "system": platform.system(),
            "release": platform.release(),
            "machine": platform.machine(),
            "platform": platform.platform(),
            "python": sys.version.replace("\n", " "),
        },
        "toolchain": {
            "cmake": run_probe(["cmake", "--version"]),
            "compiler": run_probe([cxx, "--version"]),
            "ctest": run_probe(["ctest", "--version"]),
            "git": run_probe(["git", "--version"]),
        },
        "environment": {
            "cxx": cxx,
            "build_type": args.build_type,
            "warnings_as_errors": args.warnings_as_errors,
            "sanitizers_enabled": args.sanitizers_enabled,
            "coverage_enabled": args.coverage_enabled,
            "source_date_epoch": os.environ.get("SOURCE_DATE_EPOCH"),
            "github_run_id": os.environ.get("GITHUB_RUN_ID"),
            "github_run_attempt": os.environ.get("GITHUB_RUN_ATTEMPT"),
            "github_workflow": os.environ.get("GITHUB_WORKFLOW"),
        },
        "inputs": file_manifest(),
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--verification-status",
        choices=("passed", "failed", "not-run"),
        default="not-run",
    )
    parser.add_argument("--test-command", action="append", default=[])
    parser.add_argument("--note", default="")
    parser.add_argument("--build-type", default="unknown")
    parser.add_argument("--warnings-as-errors", action="store_true")
    parser.add_argument("--sanitizers-enabled", action="store_true")
    parser.add_argument("--coverage-enabled", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output = args.output
    if not output.is_absolute():
        output = REPO_ROOT / output

    output.parent.mkdir(parents=True, exist_ok=True)
    manifest = build_manifest(args)
    output.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"Wrote provenance manifest: {output.relative_to(REPO_ROOT)}")
    print(f"Hashed inputs: {len(manifest['inputs'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
