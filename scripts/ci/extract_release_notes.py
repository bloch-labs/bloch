#!/usr/bin/env python3
"""Extract the changelog section for a release tag and write it to a file.

Usage: extract_release_notes.py <tag> <version> <output-path>
- tag: full tag name (e.g. v1.2.3)
- version: semantic version without leading 'v' (e.g. 1.2.3)
- output-path: where to write the Markdown notes
"""

from __future__ import annotations

import os
import re
import sys
from pathlib import Path


def build_pattern(tag: str, version: str) -> re.Pattern[str]:
    escaped_tag = re.escape(tag)
    escaped_version = re.escape(version)
    # Match headings like "## v1.2.3" or "## [v1.2.3]" and fall back to bare version
    pattern = rf"^## \[?{escaped_tag}\]?|^## \[?{escaped_version}\]?"
    return re.compile(pattern, re.MULTILINE)


def extract_section(changelog: str, pattern: re.Pattern[str]) -> str | None:
    match = pattern.search(changelog)
    if not match:
        return None
    start = match.end()
    remainder = changelog[start:]
    next_heading = re.search(r"^## ", remainder, re.MULTILINE)
    section = remainder[: next_heading.start()] if next_heading else remainder
    section = section.strip()
    return section if section else None


def main() -> int:
    if len(sys.argv) != 4:
        sys.stderr.write("Usage: extract_release_notes.py <tag> <version> <output>\n")
        return 1

    tag, version, output_path = sys.argv[1:4]
    changelog_path = Path("CHANGELOG.md")
    if not changelog_path.exists():
        sys.stderr.write("CHANGELOG.md not found\n")
        return 1

    changelog = changelog_path.read_text(encoding="utf-8")
    notes = extract_section(changelog, build_pattern(tag, version))
    if notes is None:
        notes = f"Bloch {tag}\n\nSee CHANGELOG.md for details."

    output = Path(output_path)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(notes, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
