#!/usr/bin/env python3
"""Classify Git working-tree changes as source or generated evidence."""

from __future__ import annotations

import subprocess
from dataclasses import dataclass
from pathlib import Path

GENERATED_PREFIXES = (
    "reports/",
    "dist/",
)


@dataclass(frozen=True)
class RepositoryState:
    dirty_paths: tuple[str, ...]
    source_dirty_paths: tuple[str, ...]
    generated_dirty_paths: tuple[str, ...]

    @property
    def dirty(self) -> bool:
        return bool(self.dirty_paths)

    @property
    def source_dirty(self) -> bool:
        return bool(self.source_dirty_paths)


def _normalize_porcelain_path(raw: str) -> str:
    value = raw.strip()
    if " -> " in value:
        value = value.split(" -> ", 1)[1]
    if value.startswith('"') and value.endswith('"'):
        value = value[1:-1]
    return value


def collect_repository_state(repo_root: Path) -> RepositoryState:
    completed = subprocess.run(
        ["git", "status", "--porcelain", "--untracked-files=all"],
        cwd=repo_root,
        check=False,
        capture_output=True,
        text=True,
        timeout=30,
    )
    if completed.returncode != 0:
        raise RuntimeError(completed.stderr.strip() or "git status failed")

    paths: list[str] = []
    for line in completed.stdout.splitlines():
        if len(line) < 4:
            continue
        path = _normalize_porcelain_path(line[3:])
        if path:
            paths.append(path)

    unique = tuple(sorted(set(paths)))
    generated = tuple(
        path
        for path in unique
        if any(path == prefix.rstrip("/") or path.startswith(prefix) for prefix in GENERATED_PREFIXES)
    )
    generated_set = set(generated)
    source = tuple(path for path in unique if path not in generated_set)
    return RepositoryState(unique, source, generated)
