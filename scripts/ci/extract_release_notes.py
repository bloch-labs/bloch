#!/usr/bin/env python3
"""Extract a changelog section for a version and write it to a workflow output.

Usage:
    python3 scripts/ci/extract_release_notes.py --version v1.2.3 --changelog CHANGELOG.md --output "$GITHUB_OUTPUT" --key body

The version may include or omit the leading 'v'. The script finds the heading
matching "## [<version>]" (without the leading v). If the heading is missing,
an empty string is written to the output key so that the workflow can fall back gracefully.
"""
from __future__ import annotations

import argparse
import os
import re
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Extract release notes from changelog")
    parser.add_argument("--version", required=True, help="Version string (e.g. v1.2.3 or 1.2.3)")
    parser.add_argument("--changelog", default="CHANGELOG.md", help="Path to changelog file")
    parser.add_argument("--output", default="", help="Path to GitHub Actions output file")
    parser.add_argument("--key", default="body", help="Name of the GitHub Actions output key")
    return parser.parse_args()


def extract_notes(changelog: Path, version: str) -> str:
    if not changelog.exists():
        return ""

    content = changelog.read_text(encoding="utf-8")
    cleaned_version = version.lstrip("v").strip()
    heading = re.compile(rf"^## \\[{{re.escape(cleaned_version)}}\\].*$", re.MULTILINE)
    match = heading.search(content)
    if not match:
        return ""

    start = match.end()
    remainder = content[start:]
    next_heading_index = remainder.find("\n## ")
    section = remainder[:next_heading_index].strip() if next_heading_index != -1 else remainder.strip()
    return section.strip()


def write_output(output_path: Path, key: str, value: str) -> None:
    if not output_path:
        print(value)
        return
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("a", encoding="utf-8") as fh:
        fh.write(f"{key}<<'__EOF__'\n")
        fh.write(value + "\n")
        fh.write("__EOF__\n")


def main() -> None:
    args = parse_args()
    notes = extract_notes(Path(args.changelog), args.version)
    output_path = Path(args.output) if args.output else None
    write_output(output_path, args.key, notes)


if __name__ == "__main__":
    main()
