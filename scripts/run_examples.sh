#!/usr/bin/env bash

# Run all examples as an acceptance test.

if [ -z "${BASH_VERSION:-}" ]; then
  echo "This script requires bash (macOS/Linux)." >&2
  exit 1
fi

set -euo pipefail

# Match the local build script: use colours only for an interactive terminal,
# and honour NO_COLOR for users and CI systems that prefer plain output.
if [[ -t 1 && "${NO_COLOR:-}" != "1" ]]; then
  RED="\033[31m"
  GREEN="\033[32m"
  RESET="\033[0m"
else
  RED=""
  GREEN=""
  RESET=""
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BLOCH_BIN="${1:-${BUILD_DIR:-${ROOT_DIR}/build}/bin/bloch}"
EXAMPLE_TIMEOUT_SECONDS="${EXAMPLE_TIMEOUT_SECONDS:-30}"

if [[ ! -x "${BLOCH_BIN}" ]]; then
  echo "Bloch executable not found or not executable: ${BLOCH_BIN}" >&2
  echo "Build Bloch first or pass the executable path as the first argument." >&2
  exit 1
fi
if [[ ! "${EXAMPLE_TIMEOUT_SECONDS}" =~ ^[1-9][0-9]*$ ]]; then
  echo "EXAMPLE_TIMEOUT_SECONDS must be a positive integer." >&2
  exit 1
fi

# Resolve the executable before changing to the temporary working tree.
BLOCH_BIN="$(cd "$(dirname "${BLOCH_BIN}")" && pwd)/$(basename "${BLOCH_BIN}")"

TEMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/bloch-examples.XXXXXX")"
trap 'rm -rf "${TEMP_DIR}"' EXIT
cp -R "${ROOT_DIR}/examples" "${TEMP_DIR}/examples"

EXAMPLES=()
while IFS= read -r example; do
  EXAMPLES+=("${example}")
done < <(
  {
    find "${TEMP_DIR}/examples" -maxdepth 1 -type f -name '*.bloch'
    find "${TEMP_DIR}/examples" -mindepth 2 -type f -name 'main.bloch'
  } | sort
)

if [[ ${#EXAMPLES[@]} -eq 0 ]]; then
  echo "No runnable examples found." >&2
  exit 1
fi

run_example() {
  local example="$1"
  local output_file="$2"
  local pid
  local started_at="${SECONDS}"

  "${BLOCH_BIN}" "${example}" >"${output_file}" 2>&1 &
  pid=$!

  while kill -0 "${pid}" 2>/dev/null; do
    if ((SECONDS - started_at >= EXAMPLE_TIMEOUT_SECONDS)); then
      kill "${pid}" 2>/dev/null || true
      wait "${pid}" 2>/dev/null || true
      echo "Timed out after ${EXAMPLE_TIMEOUT_SECONDS} seconds." >>"${output_file}"
      return 124
    fi
    sleep 1
  done

  wait "${pid}"
}

failures=0
for example in "${EXAMPLES[@]}"; do
  relative_path="${example#${TEMP_DIR}/}"
  output_file="${TEMP_DIR}/example-output.txt"
  printf '  RUN  %s\n' "${relative_path}"

  if run_example "${example}" "${output_file}"; then
    printf '  %bPASS%b %s\n' "${GREEN}" "${RESET}" "${relative_path}"
  else
    status=$?
    failures=$((failures + 1))
    printf '  %bFAIL%b %s (exit %d)\n' \
      "${RED}" "${RESET}" "${relative_path}" "${status}" >&2
    sed 's/^/       /' "${output_file}" >&2
  fi
done

if [[ ${failures} -ne 0 ]]; then
  printf '%b%d example acceptance test(s) failed.%b\n' \
    "${RED}" "${failures}" "${RESET}" >&2
  exit 1
fi

printf '%bAll %d example acceptance tests passed.%b\n' \
  "${GREEN}" "${#EXAMPLES[@]}" "${RESET}"