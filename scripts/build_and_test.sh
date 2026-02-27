#!/usr/bin/env bash

# Build and test Bloch on macOS/Linux.
# Runs:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build --parallel
#   ctest --test-dir build --output-on-failure

if [ -z "${BASH_VERSION:-}" ]; then
  echo "This script requires bash (macOS/Linux)." >&2
  exit 1
fi

set -euo pipefail

if [[ "$(uname -s)" != "Linux" && "$(uname -s)" != "Darwin" ]]; then
  echo "Unsupported OS: $(uname -s). This script is for macOS/Linux only." >&2
  exit 1
fi

# Pretty output helpers
if [[ -t 1 && "${NO_COLOR:-}" != "1" ]]; then
  RED="\033[31m"; GREEN="\033[32m"; YELLOW="\033[33m"; BLUE="\033[34m"; BOLD="\033[1m"; RESET="\033[0m"
else
  RED=""; GREEN=""; YELLOW=""; BLUE=""; BOLD=""; RESET=""
fi

step()    { printf "%b➜%b %s\n" "$BLUE" "$RESET" "$*"; }
success() { printf "%b✓%b %s\n" "$GREEN" "$RESET" "$*"; }
warn()    { printf "%b!%b %s\n" "$YELLOW" "$RESET" "$*" >&2; }
error()   { printf "%b✗%b %s\n" "$RED" "$RESET" "$*" >&2; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"
CONFIG="${CONFIG:-Release}"

detect_jobs() {
  if command -v nproc >/dev/null 2>&1; then
    nproc
  elif command -v sysctl >/dev/null 2>&1; then
    sysctl -n hw.ncpu 2>/dev/null || echo 4
  else
    echo 4
  fi
}
JOBS="${JOBS:-$(detect_jobs)}"

require_cmd() {
  command -v "$1" >/dev/null 2>&1 || { error "Required command '$1' not found in PATH."; exit 1; }
}

require_cmd cmake
require_cmd ctest

cache_internal_value() {
  local key="$1"
  local cache_file="$2"
  local line

  line="$(grep -E "^${key}:INTERNAL=" "${cache_file}" 2>/dev/null || true)"
  printf "%s\n" "${line#*=}"
}

cmake_supports_fresh() {
  cmake --help 2>/dev/null | grep -q -- "--fresh"
}

CONFIGURE_ARGS=(-S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${CONFIG}")

if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  CACHED_SOURCE_DIR="$(cache_internal_value CMAKE_HOME_DIRECTORY "${BUILD_DIR}/CMakeCache.txt")"
  CACHED_BUILD_DIR="$(cache_internal_value CMAKE_CACHEFILE_DIR "${BUILD_DIR}/CMakeCache.txt")"

  if [[ "${CACHED_SOURCE_DIR}" != "${ROOT_DIR}" || "${CACHED_BUILD_DIR}" != "${BUILD_DIR}" ]]; then
    warn "Detected stale CMake cache for a different checkout path."
    warn "Cached source: ${CACHED_SOURCE_DIR:-unknown}"
    warn "Cached build:  ${CACHED_BUILD_DIR:-unknown}"

    if cmake_supports_fresh; then
      warn "Reconfiguring with 'cmake --fresh' to rebuild CMake metadata."
      CONFIGURE_ARGS=(--fresh "${CONFIGURE_ARGS[@]}")
    else
      warn "Removing stale CMake cache files before reconfiguring."
      cmake -E rm -f \
        "${BUILD_DIR}/CMakeCache.txt" \
        "${BUILD_DIR}/CTestTestfile.cmake" \
        "${BUILD_DIR}/cmake_install.cmake" \
        "${BUILD_DIR}/compile_commands.json"
      if [[ -d "${BUILD_DIR}/CMakeFiles" ]]; then
        cmake -E remove_directory "${BUILD_DIR}/CMakeFiles"
      fi
    fi
  fi
fi

step "Configuring CMake (${CONFIG}) in ${BUILD_DIR}"
cmake "${CONFIGURE_ARGS[@]}"

step "Building (parallel: ${JOBS})"
cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

step "Running tests"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

success "Build and tests completed successfully."
