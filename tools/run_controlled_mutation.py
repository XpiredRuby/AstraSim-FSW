#!/usr/bin/env python3
"""Verify that the command-packet tests kill a controlled CRC acceptance defect."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
TARGET_RELATIVE = Path("fsw/src/command_packet.cpp")
ORIGINAL = "if (actual_crc != expected_crc) {"
MUTATED = "if (actual_crc != expected_crc && bytes.empty()) {"


def run(command: list[str], cwd: Path) -> subprocess.CompletedProcess[str]:
    print("+", " ".join(command))
    return subprocess.run(command, cwd=cwd, check=False, text=True)


def copy_repository(destination: Path) -> Path:
    source_copy = destination / "source"
    shutil.copytree(
        REPO_ROOT,
        source_copy,
        ignore=shutil.ignore_patterns(
            ".git",
            "build",
            "build-*",
            "__pycache__",
            "*.pyc",
            "reports/latest",
            "dist",
        ),
    )
    return source_copy


def apply_mutation(source_copy: Path) -> None:
    target = source_copy / TARGET_RELATIVE
    content = target.read_text(encoding="utf-8")
    count = content.count(ORIGINAL)
    if count != 1:
        raise RuntimeError(
            f"Expected exactly one CRC guard to mutate, found {count}; target may have changed."
        )
    target.write_text(content.replace(ORIGINAL, MUTATED, 1), encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--keep-workdir",
        action="store_true",
        help="Copy the temporary mutation workspace to mutation-workdir for inspection.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    with tempfile.TemporaryDirectory(prefix="astra-os-mutation-") as temporary:
        workspace = Path(temporary)
        source_copy = copy_repository(workspace)
        apply_mutation(source_copy)
        build_dir = workspace / "build"

        configure = run(
            [
                "cmake",
                "-S",
                str(source_copy),
                "-B",
                str(build_dir),
                "-DCMAKE_BUILD_TYPE=Debug",
                "-DASTRA_WARNINGS_AS_ERRORS=ON",
            ],
            workspace,
        )
        if configure.returncode != 0:
            print("ERROR: controlled mutation could not be configured.")
            return configure.returncode

        build = run(
            ["cmake", "--build", str(build_dir), "--target", "astra_command_tests", "--parallel"],
            workspace,
        )
        if build.returncode != 0:
            print("ERROR: controlled mutation did not compile; this is not a valid test-effectiveness result.")
            return build.returncode

        test = run(
            [
                "ctest",
                "--test-dir",
                str(build_dir),
                "--tests-regex",
                "^command_packet_tests$",
                "--output-on-failure",
            ],
            workspace,
        )

        if args.keep_workdir:
            retained = REPO_ROOT / "mutation-workdir"
            if retained.exists():
                shutil.rmtree(retained)
            shutil.copytree(workspace, retained)
            print(f"Retained workspace: {retained}")

        if test.returncode == 0:
            print("MUTATION SURVIVED: command-packet tests accepted a disabled CRC rejection path.")
            return 1

        print("MUTATION KILLED: command-packet tests detected the disabled CRC rejection path.")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
