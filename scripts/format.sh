#!/usr/bin/env bash

set -euo pipefail

# Resolve repository root (directory containing this script's parent).
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)

# Allow overriding the binary via CLANG_FORMAT env var; default to clang-format-14.
CLANG_FORMAT_BIN=${CLANG_FORMAT:-clang-format-14}

usage() {
    cat <<'EOF'
Usage: scripts/format.sh [--check]

Formats all C/C++ sources tracked by the repository's .clang-format file.

Options:
  --check    Run clang-format in diff mode (no files are modified); exits non-zero
             if any file would be reformatted. Mirrors the CI formatting check.

Environment variables:
  CLANG_FORMAT    Override the clang-format binary (default: clang-format-14)
EOF
}

if [[ ${1:-} == "-h" || ${1:-} == "--help" ]]; then
    usage
    exit 0
fi

if ! command -v "$CLANG_FORMAT_BIN" >/dev/null 2>&1; then
    echo "Error: '$CLANG_FORMAT_BIN' not found. Install clang-format-14 or set CLANG_FORMAT." >&2
    exit 1
fi

cd "$REPO_ROOT"

mapfile -d '' sources < <(find . -regex '.*\.\(cpp\|hpp\|c\|h\)' -print0)

if [[ ${#sources[@]} -eq 0 ]]; then
    echo "No C/C++ source files found."
    exit 0
fi

if [[ ${1:-} == "--check" ]]; then
    echo "Running clang-format check using '$CLANG_FORMAT_BIN'."
    if ! printf '%s\0' "${sources[@]}" \
        | xargs -0 -r "$CLANG_FORMAT_BIN" -style=file -n --Werror; then
        echo "Formatting issues detected. Rerun without --check to fix." >&2
        exit 1
    fi
    echo "Formatting looks good."
else
    echo "Formatting sources using '$CLANG_FORMAT_BIN'..."
    printf '%s\0' "${sources[@]}" | xargs -0 -r "$CLANG_FORMAT_BIN" -style=file -i
    echo "Done."
fi
