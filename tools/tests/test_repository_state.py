from __future__ import annotations

import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.repository_state import collect_repository_state


class RepositoryStateTests(unittest.TestCase):
    def make_repo(self) -> Path:
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        root = Path(directory.name)
        subprocess.run(["git", "init", "-q"], cwd=root, check=True)
        subprocess.run(["git", "config", "user.name", "Test"], cwd=root, check=True)
        subprocess.run(
            ["git", "config", "user.email", "test@example.invalid"],
            cwd=root,
            check=True,
        )
        (root / "source.txt").write_text("clean\n", encoding="utf-8")
        (root / "reports").mkdir()
        (root / "reports" / "result.txt").write_text("pass\n", encoding="utf-8")
        subprocess.run(["git", "add", "."], cwd=root, check=True)
        subprocess.run(["git", "commit", "-qm", "baseline"], cwd=root, check=True)
        return root

    def test_clean_repository(self) -> None:
        state = collect_repository_state(self.make_repo())
        self.assertFalse(state.dirty)
        self.assertFalse(state.source_dirty)

    def test_generated_report_change_is_not_source_dirty(self) -> None:
        root = self.make_repo()
        (root / "reports" / "result.txt").write_text("updated\n", encoding="utf-8")
        state = collect_repository_state(root)
        self.assertTrue(state.dirty)
        self.assertFalse(state.source_dirty)
        self.assertEqual(state.generated_dirty_paths, ("reports/result.txt",))

    def test_source_change_is_source_dirty(self) -> None:
        root = self.make_repo()
        (root / "source.txt").write_text("changed\n", encoding="utf-8")
        state = collect_repository_state(root)
        self.assertTrue(state.source_dirty)
        self.assertEqual(state.source_dirty_paths, ("source.txt",))

    def test_untracked_generated_report_is_permitted(self) -> None:
        root = self.make_repo()
        (root / "reports" / "new.txt").write_text("new\n", encoding="utf-8")
        state = collect_repository_state(root)
        self.assertFalse(state.source_dirty)
        self.assertIn("reports/new.txt", state.generated_dirty_paths)


if __name__ == "__main__":
    unittest.main()
