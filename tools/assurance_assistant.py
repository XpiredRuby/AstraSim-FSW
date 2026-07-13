#!/usr/bin/env python3
"""Permissioned, deterministic interface for ASTRA-OS assurance retrieval and tools."""

from __future__ import annotations

import argparse
import json
import subprocess
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_POLICY = REPO_ROOT / "config" / "assurance_assistant_policy.json"
DEFAULT_AUDIT_LOG = REPO_ROOT / "reports" / "latest" / "assurance_assistant_audit.jsonl"
MAX_READ_BYTES = 65536


@dataclass(frozen=True)
class Decision:
    allowed: bool
    action: str
    target: str
    reason: str
    command: tuple[str, ...] = ()
    resolved_path: str | None = None

    def to_dict(self) -> dict[str, object]:
        return {
            "allowed": self.allowed,
            "action": self.action,
            "target": self.target,
            "reason": self.reason,
            "command": list(self.command),
            "resolved_path": self.resolved_path,
        }


def load_policy(path: Path) -> dict[str, Any]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise ValueError(f"unable to read assistant policy {path}: {error}") from error

    if payload.get("schema") != "astra-os.assurance-assistant-policy.v1":
        raise ValueError(f"unexpected assistant policy schema in {path}")
    if not isinstance(payload.get("approved_read_roots"), list):
        raise ValueError("approved_read_roots must be a list")
    if not isinstance(payload.get("allowlisted_tools"), dict):
        raise ValueError("allowlisted_tools must be an object")
    if not isinstance(payload.get("explicitly_denied_actions"), list):
        raise ValueError("explicitly_denied_actions must be a list")
    return payload


def _is_relative_to(path: Path, root: Path) -> bool:
    try:
        path.relative_to(root)
    except ValueError:
        return False
    return True


def resolve_approved_read_path(
    target: str,
    policy: dict[str, Any],
    repo_root: Path = REPO_ROOT,
) -> Path | None:
    candidate = Path(target)
    if candidate.is_absolute():
        resolved = candidate.resolve(strict=False)
    else:
        resolved = (repo_root / candidate).resolve(strict=False)

    repository = repo_root.resolve()
    if not _is_relative_to(resolved, repository):
        return None

    for root_text in policy["approved_read_roots"]:
        approved = (repository / str(root_text)).resolve(strict=False)
        if resolved == approved or _is_relative_to(resolved, approved):
            return resolved
    return None


def decide_request(
    request: dict[str, Any],
    policy: dict[str, Any],
    repo_root: Path = REPO_ROOT,
) -> Decision:
    action = str(request.get("action", "")).strip()
    target = str(request.get("target", "")).strip()

    if not action:
        return Decision(False, action, target, "Request action is required")

    denied_actions = {str(value) for value in policy["explicitly_denied_actions"]}
    if action in denied_actions:
        return Decision(False, action, target, f"Action {action!r} is explicitly denied")

    if action == "read":
        if not target:
            return Decision(False, action, target, "Read target is required")
        resolved = resolve_approved_read_path(target, policy, repo_root)
        if resolved is None:
            return Decision(False, action, target, "Path is outside approved project sources")
        if not resolved.is_file():
            return Decision(False, action, target, "Approved read target is not a file")
        return Decision(
            True,
            action,
            target,
            "Read target is within an approved project source root",
            resolved_path=str(resolved),
        )

    if action == "run":
        tools = policy["allowlisted_tools"]
        if target not in tools:
            return Decision(False, action, target, "Verification tool is not allow-listed")
        command_value = tools[target]
        if not isinstance(command_value, list) or not all(
            isinstance(item, str) and item for item in command_value
        ):
            return Decision(False, action, target, "Allow-listed tool definition is invalid")
        return Decision(
            True,
            action,
            target,
            "Verification tool exactly matches the allow-list",
            command=tuple(command_value),
        )

    return Decision(False, action, target, "Only read and allow-listed run actions are supported")


def append_audit(
    path: Path,
    request: dict[str, Any],
    decision: Decision,
    executed: bool,
    return_code: int | None,
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    record = {
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "request": request,
        "decision": decision.to_dict(),
        "executed": executed,
        "return_code": return_code,
    }
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(record, sort_keys=True) + "\n")


def execute_decision(
    decision: Decision,
    repo_root: Path = REPO_ROOT,
) -> tuple[int, str]:
    if not decision.allowed:
        return 2, decision.reason

    if decision.action == "read":
        assert decision.resolved_path is not None
        path = Path(decision.resolved_path)
        data = path.read_bytes()
        if len(data) > MAX_READ_BYTES:
            data = data[:MAX_READ_BYTES]
            suffix = b"\n[TRUNCATED BY ASSURANCE ASSISTANT]\n"
        else:
            suffix = b""
        return 0, (data + suffix).decode("utf-8", errors="replace")

    if decision.action == "run":
        completed = subprocess.run(
            list(decision.command),
            cwd=repo_root,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        return completed.returncode, completed.stdout

    return 2, "Unsupported approved action"


def parse_request(text: str) -> dict[str, Any]:
    try:
        request = json.loads(text)
    except json.JSONDecodeError as error:
        raise ValueError(f"request must be valid JSON: {error}") from error
    if not isinstance(request, dict):
        raise ValueError("request JSON must be an object")
    return request


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--request", required=True, help="JSON object with action and target")
    parser.add_argument("--policy", type=Path, default=DEFAULT_POLICY)
    parser.add_argument("--audit-log", type=Path, default=DEFAULT_AUDIT_LOG)
    parser.add_argument(
        "--execute",
        action="store_true",
        help="Execute an approved read or verification tool after authorization",
    )
    args = parser.parse_args()

    policy_path = args.policy if args.policy.is_absolute() else REPO_ROOT / args.policy
    audit_path = args.audit_log if args.audit_log.is_absolute() else REPO_ROOT / args.audit_log

    try:
        request = parse_request(args.request)
        policy = load_policy(policy_path)
        decision = decide_request(request, policy)
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}")
        return 2

    return_code: int | None = None
    output = ""
    if args.execute and decision.allowed:
        try:
            return_code, output = execute_decision(decision)
        except OSError as error:
            return_code = 1
            output = f"ERROR: {error}"

    append_audit(
        audit_path,
        request,
        decision,
        executed=args.execute and decision.allowed,
        return_code=return_code,
    )

    print(json.dumps(decision.to_dict(), indent=2, sort_keys=True))
    if output:
        print(output, end="" if output.endswith("\n") else "\n")

    if not decision.allowed:
        return 2
    if args.execute and return_code is not None:
        return return_code
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
