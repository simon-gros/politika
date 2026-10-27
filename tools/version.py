#!/usr/bin/env python3
"""Canonical POLITIKA version validation and build identity generation.

Standard-library only. No engine installation is required.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "version.json"
KEYS = ("game_version", "network_protocol", "save_schema",
        "persistence_schema", "world_schema")
SEMVER_RE = re.compile(
    r"^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)"
    r"(?:-((?:0|[1-9]\d*|\d*[A-Za-z-][0-9A-Za-z-]*)"
    r"(?:\.(?:0|[1-9]\d*|\d*[A-Za-z-][0-9A-Za-z-]*))*))?"
    r"(?:\+([0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?$"
)


def parse_manifest(path: Path = MANIFEST) -> dict:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(f"cannot read version manifest {path}: {exc}") from exc
    if not isinstance(data, dict):
        raise ValueError("version.json must contain a JSON object")
    missing = set(KEYS) - data.keys()
    if missing:
        raise ValueError("missing version fields: " + ", ".join(sorted(missing)))
    if not isinstance(data["game_version"], str) or not SEMVER_RE.fullmatch(data["game_version"]):
        raise ValueError(f"game_version is not valid SemVer: {data['game_version']!r}")
    for key in KEYS[1:]:
        value = data[key]
        if type(value) is not int or value < 1:
            raise ValueError(f"{key} must be a positive integer")
    return data


def git(*args: str) -> str | None:
    try:
        return subprocess.run(
            ["git", *args], cwd=ROOT, text=True, check=True,
            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL
        ).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return None


def build_identity() -> dict:
    build = os.environ.get("POLITIKA_BUILD_NUMBER") or os.environ.get("GITHUB_RUN_NUMBER") or "local"
    sha = os.environ.get("GITHUB_SHA", "").strip() or git("rev-parse", "HEAD") or "unknown"
    status = git("status", "--porcelain")
    dirty = status is not None and bool(status)
    return {
        "build_number": str(build),
        "git_revision": sha[:12] + ("-dirty" if dirty else ""),
        "git_dirty": dirty,
    }


def current_tag() -> str | None:
    ref = os.environ.get("GITHUB_REF", "")
    if os.environ.get("GITHUB_REF_TYPE") == "tag" or ref.startswith("refs/tags/"):
        return os.environ.get("GITHUB_REF_NAME") or ref.removeprefix("refs/tags/")
    return None


def validate(*, release: bool = False, tag: str | None = None) -> dict:
    data = parse_manifest()
    observed_tag = tag if tag is not None else current_tag()
    if observed_tag and observed_tag != "v" + data["game_version"]:
        raise ValueError(f"tag {observed_tag!r} does not match v{data['game_version']}")
    if release:
        identity = build_identity()
        if not observed_tag:
            raise ValueError("release builds require an exact vVERSION tag")
        if git("rev-parse", "--is-inside-work-tree") != "true":
            raise ValueError("release builds require a Git checkout")
        if identity["git_dirty"]:
            raise ValueError("release builds require a clean Git worktree")
        if identity["git_revision"] == "unknown":
            raise ValueError("release builds require a Git revision")
        if git("rev-parse", "HEAD") != git("rev-list", "-n", "1", observed_tag):
            raise ValueError("release tag must resolve to the checked-out commit")
    return data


def generated_header(data: dict, identity: dict) -> str:
    def quoted(value: str) -> str:
        return json.dumps(value, ensure_ascii=True)
    return "\n".join([
        "// Generated from version.json by tools/version.py. Do not edit.",
        "#pragma once",
        "#include <cstdint>",
        "",
        "namespace politika::version {",
        f"inline constexpr char GAME_VERSION[] = {quoted(data['game_version'])};",
        f"inline constexpr std::int64_t NETWORK_PROTOCOL = {data['network_protocol']};",
        f"inline constexpr std::int64_t SAVE_SCHEMA = {data['save_schema']};",
        f"inline constexpr std::int64_t PERSISTENCE_SCHEMA = {data['persistence_schema']};",
        f"inline constexpr std::int64_t WORLD_SCHEMA = {data['world_schema']};",
        f"inline constexpr char BUILD_NUMBER[] = {quoted(identity['build_number'])};",
        f"inline constexpr char GIT_REVISION[] = {quoted(identity['git_revision'])};",
        "}  // namespace politika::version",
        "",
    ])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("print", help="Print the canonical game version")
    check = sub.add_parser("check", help="Validate the manifest and Git tag if present")
    check.add_argument("--release", action="store_true")
    check.add_argument("--tag", help="Explicit tag (for release verification)")
    generate = sub.add_parser("generate", help="Generate a C++ build identity header")
    generate.add_argument("--release", action="store_true")
    generate.add_argument("--tag")
    generate.add_argument("--output", default="build/generated/PolitikaVersion.generated.h")
    identity = sub.add_parser("identity", help="Print a machine-readable build identity")
    identity.add_argument("--release", action="store_true")
    identity.add_argument("--tag")
    args = parser.parse_args()

    if args.command == "print":
        print(parse_manifest()["game_version"])
        return 0
    data = validate(release=getattr(args, "release", False), tag=getattr(args, "tag", None))
    build = build_identity()
    if args.command == "check":
        print(f"POLITIKA {data['game_version']} · build {build['build_number']} · "
              f"git {build['git_revision']} · protocol {data['network_protocol']} · "
              f"save {data['save_schema']} · persistence {data['persistence_schema']} · "
              f"world {data['world_schema']}")
    elif args.command == "identity":
        print(json.dumps({**data, **build}, indent=2, sort_keys=True))
    elif args.command == "generate":
        output = Path(args.output)
        if not output.is_absolute():
            output = ROOT / output
        content = generated_header(data, build)
        output.parent.mkdir(parents=True, exist_ok=True)
        if not output.exists() or output.read_text(encoding="utf-8") != content:
            output.write_text(content, encoding="utf-8")
        print(output)
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except ValueError as exc:
        print(f"version error: {exc}", file=sys.stderr)
        sys.exit(2)
