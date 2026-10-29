#!/usr/bin/env python3
"""Validate POLITIKA's archival CSV schemas and indexed references.

Run from any working directory:
    python3 Archive/tools/validate_catalogues.py

Uses only the Python standard library. Validates metadata/index consistency,
not the historical truth, image licence or live availability of source URLs.
"""

from __future__ import annotations

import csv
import re
import sys
from collections import Counter
from pathlib import Path

ARCHIVE = Path(__file__).resolve().parents[1]
SCHEMA = {
    "CATALOG.csv": ("ref_id", 22),
    "MAP_CATALOG.csv": ("map_id", 17),
    "POLITICAL_EVENTS.csv": ("ref_id", 18),
}


def load_csv(name: str, key: str, expected_fields: int, problems: list[str]):
    path = ARCHIVE / name
    try:
        with path.open(encoding="utf-8-sig", newline="") as file:
            reader = csv.DictReader(file)
            columns = reader.fieldnames or []
            if len(columns) != expected_fields or key not in columns:
                problems.append(
                    f"{name}: expected {expected_fields} named columns including {key}; "
                    f"found {len(columns)}"
                )
            records = list(reader)
    except (OSError, UnicodeError, csv.Error) as exc:
        problems.append(f"{name}: failed to read CSV: {exc}")
        return [], set()

    identifiers = []
    for line_no, record in enumerate(records, start=2):
        if None in record or any(value is None for value in record.values()):
            problems.append(f"{name}:{line_no}: malformed number of CSV columns")
        record_id = record.get(key) or ""
        if not record_id.startswith("POL-"):
            problems.append(f"{name}:{line_no}: missing or non-POL identifier: {record_id!r}")
        identifiers.append(record_id)
        if name in ("CATALOG.csv", "MAP_CATALOG.csv"):
            url_field = "source_url"
            url = record.get(url_field) or ""
            if not url.startswith(("https://", "http://")):
                problems.append(f"{name}:{line_no}: source_url is absent or invalid")

    duplicates = [k for k, count in Counter(identifiers).items() if count > 1]
    if duplicates:
        problems.append(f"{name}: duplicate IDs: {', '.join(sorted(duplicates))}")
    return records, set(identifiers)


def markdown_ids(path: Path, prefix: str, problems: list[str]) -> list[str]:
    try:
        text = path.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as exc:
        problems.append(f"{path.relative_to(ARCHIVE)}: cannot read: {exc}")
        return []
    return re.findall(r"^\| `(" + re.escape(prefix) + r"[^\x60]+)\x60 \|", text, re.MULTILINE)


def check_sets(label: str, expected: set[str], actual: set[str], problems: list[str]):
    missing = sorted(expected - actual)
    extras = sorted(actual - expected)
    if missing:
        problems.append(f"{label}: {len(missing)} missing IDs: {', '.join(missing[:8])}")
    if extras:
        problems.append(f"{label}: {len(extras)} unexpected IDs: {', '.join(extras[:8])}")


def main() -> int:
    problems: list[str] = []
    master_rows, master = load_csv("CATALOG.csv", *SCHEMA["CATALOG.csv"], problems)
    map_rows, maps = load_csv("MAP_CATALOG.csv", *SCHEMA["MAP_CATALOG.csv"], problems)
    political_rows, political = load_csv(
        "POLITICAL_EVENTS.csv", *SCHEMA["POLITICAL_EVENTS.csv"], problems
    )

    check_sets("Master versus map catalogue", maps, master & maps, problems)
    check_sets("Master versus political catalogue", political, master & political, problems)

    for row in map_rows:
        if row.get("map_id") and not row["map_id"].startswith("POL-MAP-"):
            problems.append(f"MAP_CATALOG.csv: invalid map prefix: {row['map_id']}")
        status = row.get("period_status") or ""
        year = row.get("map_year") or ""
        if status.startswith("modern-") and year[:4].isdigit() and int(year[:4]) < 1990:
            problems.append(f"MAP_CATALOG.csv: modern map with pre-1990 creation year: {row['map_id']}")

    global_ids = markdown_ids(
        ARCHIVE / "media/reference-only/INDEX.md", "POL-", problems
    )
    map_ids = markdown_ids(
        ARCHIVE / "media/reference-only/maps/INDEX.md", "POL-MAP-", problems
    )
    check_sets("Unified reference index", master, set(global_ids), problems)
    check_sets("Map reference index", maps, set(map_ids), problems)

    for label, ids in (("global index", global_ids), ("map index", map_ids)):
        dupes = [k for k, n in Counter(ids).items() if n > 1]
        if dupes:
            problems.append(f"{label}: duplicate row identifiers: {', '.join(dupes[:8])}")

    print(
        f"Archive: {len(master_rows)} master records, "
        f"{len(map_rows)} map records, "
        f"{len(political_rows)} political event records; "
        f"{len(global_ids)} unified index entries, "
        f"{len(map_ids)} map index entries."
    )
    if problems:
        for problem in problems:
            print("FAIL:", problem, file=sys.stderr)
        return 1
    print("PASS: CSV schemas, unique identifiers and index cross-references.")
    print("NOTE: Source links, map publication dates and media rights require separate human review.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
