"""Regression tests for the POLITIKA version contract."""
from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

SCRIPT = Path(__file__).resolve().parents[1] / "tools" / "version.py"
spec = importlib.util.spec_from_file_location("politika_version", SCRIPT)
version = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(version)

BASE = {
    "game_version": "0.1.0-dev.0",
    "network_protocol": 1,
    "save_schema": 1,
    "persistence_schema": 1,
    "world_schema": 1,
}


class VersionTests(unittest.TestCase):
    def manifest(self, data):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        path = Path(tmp.name) / "version.json"
        path.write_text(json.dumps(data), encoding="utf-8")
        return path

    def test_baseline(self):
        self.assertEqual(version.parse_manifest()["game_version"], "0.1.0-dev.0")

    def test_semver_examples(self):
        for value in ("0.1.0-dev.0", "0.1.0-alpha.1", "0.1.0-beta.2",
                      "0.1.0-rc.1", "0.1.0", "0.1.1", "1.0.0",
                      "1.2.3+ci.42.gabc", "1.2.3-alpha.1+ci.7"):
            with self.subTest(value=value):
                self.assertEqual(
                    version.parse_manifest(self.manifest({**BASE, "game_version": value}))["game_version"],
                    value,
                )

    def test_bad_semver(self):
        for value in ("", "v0.1.0", "01.0.0", "1.02.3", "1.2", "1.2.3-01",
                      "1.2.3-alpha.01", "1.2.3+", "1.2.3-"):
            with self.subTest(value=value):
                with self.assertRaises(ValueError):
                    version.parse_manifest(self.manifest({**BASE, "game_version": value}))

    def test_missing_fields_and_invalid_counters(self):
        for key in BASE:
            case = BASE.copy()
            del case[key]
            with self.subTest(missing=key), self.assertRaises(ValueError):
                version.parse_manifest(self.manifest(case))
        for value in (0, -1, True, 1.2, "1", None):
            with self.subTest(counter=value), self.assertRaises(ValueError):
                version.parse_manifest(self.manifest({**BASE, "world_schema": value}))

    def test_header_contains_all_identity_fields(self):
        output = version.generated_header(BASE, {
            "build_number": "37", "git_revision": "abc123-dirty", "git_dirty": True
        })
        self.assertIn('GAME_VERSION[] = "0.1.0-dev.0"', output)
        self.assertIn("NETWORK_PROTOCOL = 1", output)
        self.assertIn("SAVE_SCHEMA = 1", output)
        self.assertIn("PERSISTENCE_SCHEMA = 1", output)
        self.assertIn("WORLD_SCHEMA = 1", output)
        self.assertIn('BUILD_NUMBER[] = "37"', output)
        self.assertIn('GIT_REVISION[] = "abc123-dirty"', output)
        self.assertEqual(output, version.generated_header(BASE, {
            "build_number": "37", "git_revision": "abc123-dirty", "git_dirty": True
        }))

    def test_tag_contract(self):
        with patch.object(version, "parse_manifest", return_value=BASE):
            self.assertEqual(version.validate(tag="v0.1.0-dev.0"), BASE)
            with self.assertRaisesRegex(ValueError, "does not match"):
                version.validate(tag="v0.2.0")
            with patch.dict("os.environ", {"GITHUB_REF": "refs/tags/v0.2.0"}, clear=True):
                with self.assertRaisesRegex(ValueError, "does not match"):
                    version.validate()

    def test_release_requires_tag_clean_checkout_and_matching_head(self):
        with patch.object(version, "parse_manifest", return_value=BASE):
            with patch.object(version, "git", return_value="true"):
                with self.assertRaisesRegex(ValueError, "require an exact"):
                    version.validate(release=True)
            with patch.object(version, "build_identity", return_value={
                "build_number": "1", "git_revision": "abc123-dirty", "git_dirty": True
            }):
                with patch.object(version, "git", return_value="true"):
                    with self.assertRaisesRegex(ValueError, "clean Git"):
                        version.validate(release=True, tag="v0.1.0-dev.0")
            with patch.object(version, "build_identity", return_value={
                "build_number": "1", "git_revision": "abc123", "git_dirty": False
            }):
                def fake_git(*args):
                    if args == ("rev-parse", "--is-inside-work-tree"):
                        return "true"
                    return "abcdef" if args == ("rev-parse", "HEAD") else "abcdef"
                with patch.object(version, "git", side_effect=fake_git):
                    self.assertEqual(version.validate(release=True, tag="v0.1.0-dev.0"), BASE)
                def wrong_tag(*args):
                    if args == ("rev-parse", "--is-inside-work-tree"):
                        return "true"
                    return "abcdef" if args == ("rev-parse", "HEAD") else "other"
                with patch.object(version, "git", side_effect=wrong_tag):
                    with self.assertRaisesRegex(ValueError, "checked-out commit"):
                        version.validate(release=True, tag="v0.1.0-dev.0")


if __name__ == "__main__":
    unittest.main()
