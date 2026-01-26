#!/usr/bin/env bash

if [ -z "${BASH_VERSION:-}" ]; then
  echo "This installer requires bash. Re-run with: curl -fsSL https://bloch-labs.com/install.sh | bash -s -- <version>" >&2
  exit 1
fi

set -euo pipefail

# Pretty output helpers
if [[ -t 1 && "${NO_COLOR:-}" != "1" ]]; then
  RED="\033[31m"; GREEN="\033[32m"; YELLOW="\033[33m"; BLUE="\033[34m"; MAGENTA="\033[35m"; CYAN="\033[36m"; BOLD="\033[1m"; DIM="\033[2m"; RESET="\033[0m"
else
  RED=""; GREEN=""; YELLOW=""; BLUE=""; MAGENTA=""; CYAN=""; BOLD=""; DIM=""; RESET=""
fi

step()    { printf "%b➜%b %s\n" "$BLUE" "$RESET" "$*"; }
success() { printf "%b✓%b %s\n" "$GREEN" "$RESET" "$*"; }
warn()    { printf "%b!%b %s\n" "$YELLOW" "$RESET" "$*" >&2; }
error()   { printf "%b✗%b %s\n" "$RED" "$RESET" "$*" >&2; }
note()    { printf "%b%s%b\n" "$DIM" "$*" "$RESET"; }

has_tty() {
  # Consider interactive if stdout or stderr is a TTY, or FORCE_INTERACTIVE is set
  [[ -t 1 || -t 2 || -n "${FORCE_INTERACTIVE:-}" ]]
}

# Bloch installer for Linux/macOS
# Usage examples:
#   curl -fsSL https://bloch-labs.com/install.sh | sh -s -- v1.0.0-rc.1
#   curl -fsSL https://bloch-labs.com/install.sh | sh -s -- latest
#   INSTALL_DIR=$HOME/.local/bin sh scripts/install.sh v1.0.0-rc.1

REPO="bloch-labs/bloch"

usage() {
  cat <<EOF
Bloch installer

Usage:
  $0 <version|latest|latest-rc>

Environment variables:
  INSTALL_DIR   Destination directory (default: /usr/local/bin or ~/.local/bin fallback)

Examples:
  $0 v1.0.0-rc.1
  $0 latest
  INSTALL_DIR=\$HOME/.local/bin $0 v1.0.0-rc.1
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

VERSION_INPUT=${1:-latest}

require_cmd() {
  command -v "$1" >/dev/null 2>&1 || { echo "Error: required command '$1' not found" >&2; exit 1; }
}

require_cmd curl
require_cmd tar

# Detect OS
UNAME_S=$(uname -s)
case "$UNAME_S" in
  Linux) OS="Linux" ;;
  Darwin) OS="macOS" ;;
  *) echo "Error: unsupported OS: $UNAME_S" >&2; exit 1 ;;
esac

# Detect ARCH mapping to GitHub runner arch naming
UNAME_M=$(uname -m)
case "$UNAME_M" in
  x86_64|amd64) ARCH="X64" ;;
  arm64|aarch64) ARCH="ARM64" ;;
  *) echo "Error: unsupported architecture: $UNAME_M" >&2; exit 1 ;;
esac

resolve_version() {
  local input=$1
  if [[ "$input" == latest ]]; then
    # Latest stable release (not prerelease)
    local tag
    tag=$(curl -fsSL "https://api.github.com/repos/${REPO}/releases/latest" | sed -n 's/^  \{0,\}"tag_name": "\(.*\)",$/\1/p' | head -n1)
    if [[ -z "$tag" ]]; then
      echo "Error: could not resolve latest release tag via GitHub API" >&2
      exit 1
    fi
    echo "$tag"
  elif [[ "$input" == latest-rc ]]; then
    # Latest prerelease; requires jq for robustness
    if command -v jq >/dev/null 2>&1; then
      curl -fsSL "https://api.github.com/repos/${REPO}/releases" | jq -r '.[] | select(.prerelease==true) | .tag_name' | head -n1
    else
      echo "Error: resolving 'latest-rc' requires 'jq'. Please install jq or pass an explicit tag (e.g., v1.0.0-rc.1)." >&2
      exit 1
    fi
  else
    echo "$input"
  fi
}

TAG=$(resolve_version "$VERSION_INPUT")
if [[ -z "$TAG" ]]; then
  echo "Error: empty tag resolved" >&2
  exit 1
fi

ASSET="bloch-${TAG}-${OS}-${ARCH}.tar.gz"
BASE_URL="https://github.com/${REPO}/releases/download/${TAG}"
ASSET_URL="${BASE_URL}/${ASSET}"
CHECKSUMS_URL="${BASE_URL}/checksums.txt"

download_checksums() {
  local url="$1" dest="$2"
  if [[ -t 1 ]]; then
    curl -fsSL -# "$url" -o "$dest"
  else
    curl -fsSL "$url" -o "$dest"
  fi
}

# Destination directory logic: prefer ~/.local/bin, then /usr/local/bin, then ~/bin
choose_dest() {
  if [[ -n "${INSTALL_DIR:-}" ]]; then
    echo "$INSTALL_DIR"
    return
  fi
  # Prefer ~/.local/bin (create if possible)
  if mkdir -p "$HOME/.local/bin" 2>/dev/null; then
    if [[ -w "$HOME/.local/bin" ]]; then
      echo "$HOME/.local/bin"
      return
    fi
  fi
  # Then /usr/local/bin if writable (no sudo)
  if [[ -d "/usr/local/bin" && -w "/usr/local/bin" ]]; then
    echo "/usr/local/bin"
    return
  fi
  # Fallback to ~/bin (create if possible)
  if mkdir -p "$HOME/bin" 2>/dev/null; then
    echo "$HOME/bin"
    return
  fi
  # Last resort
  echo "$HOME/.local/bin"
}

DEST=$(choose_dest)
mkdir -p "$DEST"
DATA_HOME=${XDG_DATA_HOME:-"$HOME/.local/share"}
STDLIB_DEST="$DATA_HOME/bloch/library"

TMPDIR=$(mktemp -d)
cleanup() { rm -rf "$TMPDIR"; }
trap cleanup EXIT

step "Installing Bloch ${TAG} for ${OS}/${ARCH} into ${DEST}"
step "Downloading: ${ASSET_URL}"
if [[ -t 1 ]]; then
  curl -fSL -# "$ASSET_URL" -o "$TMPDIR/$ASSET"
else
  curl -fSL "$ASSET_URL" -o "$TMPDIR/$ASSET"
fi

step "Downloading checksums: ${CHECKSUMS_URL}"
CHECKSUMS_PATH=""
if download_checksums "$CHECKSUMS_URL" "$TMPDIR/checksums.txt"; then
  CHECKSUMS_PATH="$TMPDIR/checksums.txt"
  CHECKSUMS_LABEL="checksums.txt"
else
  warn "checksums.txt not found; trying OS/arch-specific checksum file"
  ALT_CHECKSUMS_URL="${BASE_URL}/checksums-${OS}-${ARCH}.txt"
  if download_checksums "$ALT_CHECKSUMS_URL" "$TMPDIR/checksums-${OS}-${ARCH}.txt"; then
    CHECKSUMS_PATH="$TMPDIR/checksums-${OS}-${ARCH}.txt"
    CHECKSUMS_LABEL="checksums-${OS}-${ARCH}.txt"
  else
    warn "No checksum file found; skipping verification"
    NO_CHECKSUM=1
  fi
fi

if [[ -z "${NO_CHECKSUM:-}" ]]; then
  step "Verifying checksum (${CHECKSUMS_LABEL})"
  if command -v sha256sum >/dev/null 2>&1; then
    EXPECTED=$(grep -F "  ${ASSET}" "$CHECKSUMS_PATH" | awk '{print $1}')
    ACTUAL=$(sha256sum "$TMPDIR/$ASSET" | awk '{print $1}')
  else
    EXPECTED=$(grep -F "  ${ASSET}" "$CHECKSUMS_PATH" | awk '{print $1}')
    ACTUAL=$(shasum -a 256 "$TMPDIR/$ASSET" | awk '{print $1}')
  fi
  if [[ -z "${EXPECTED:-}" ]]; then
    warn "expected checksum not found for ${ASSET} in ${CHECKSUMS_LABEL}; skipping verification"
  elif [[ "$EXPECTED" != "$ACTUAL" ]]; then
    error "checksum mismatch for ${ASSET}"
    echo "Expected: $EXPECTED" >&2
    echo "Actual:   $ACTUAL" >&2
    exit 1
  else
    success "Checksum OK"
  fi
fi

step "Extracting"
tar -C "$TMPDIR" -xzf "$TMPDIR/$ASSET"

BLOCH_BIN_CANDIDATES=$(find "$TMPDIR" -maxdepth 5 -type f -name 'bloch')
if [[ -z "${BLOCH_BIN_CANDIDATES// }" ]]; then
  error "extracted archive did not contain a 'bloch' binary"
  exit 1
fi

candidate_count=$(printf "%s\n" "$BLOCH_BIN_CANDIDATES" | sed '/^$/d' | wc -l | tr -d '[:space:]')
if [[ "$candidate_count" != "1" ]]; then
  warn "Multiple bloch binaries found; using the first match"
fi
BLOCH_BIN=$(printf "%s\n" "$BLOCH_BIN_CANDIDATES" | sed '/^$/d' | head -n1)

chmod +x "$BLOCH_BIN"
if ! BLOCH_PROBE_OUTPUT=$("$BLOCH_BIN" --version 2>&1); then
  error "Downloaded binary failed to run on this system."
  if [[ "$BLOCH_PROBE_OUTPUT" == *"GLIBC_"* || "$BLOCH_PROBE_OUTPUT" == *"GLIBCXX_"* ]]; then
    warn "Bloch requires glibc >= 2.35 and libstdc++ >= 3.4.30."
    warn "Upgrade to a newer Linux distribution or build from source."
  fi
  echo "$BLOCH_PROBE_OUTPUT" >&2
  exit 1
fi
install -m 0755 "$BLOCH_BIN" "$DEST/bloch"

success "Installed: $DEST/bloch"
note "You can verify with: bloch --version"

# Install stdlib (Bloch sources) to a shared data directory
STDLIB_SRC=$(find "$TMPDIR" -maxdepth 5 -type d -name 'library' | head -n1)
if [[ -n "${STDLIB_SRC// }" ]]; then
  step "Installing stdlib to $STDLIB_DEST"
  mkdir -p "$STDLIB_DEST"
  cp -R "$STDLIB_SRC"/. "$STDLIB_DEST"/
  success "Stdlib installed to $STDLIB_DEST"
else
  warn "No stdlib directory found in archive. Imports like 'bloch.lang.Integer' may fail."
  warn "You can set BLOCH_STDLIB_PATH to point to a local stdlib copy."
fi

# Offer to add DEST to PATH if not already there
path_has_dest=0
case ":$PATH:" in
  *":$DEST:"*) path_has_dest=1 ;;
esac

if ! command -v bloch >/dev/null 2>&1 || [[ $path_has_dest -eq 0 ]]; then
  SHELL_NAME=$(basename "${SHELL:-}")
  if [[ "$SHELL_NAME" == "fish" ]]; then
    warn "Detected fish shell. Automatic PATH update not applied."
    echo "Add this line to ~/.config/fish/config.fish:" >&2
    echo "  set -gx PATH \"$DEST\" \$PATH" >&2
  else
    # Choose a profile file
    PROFILE=""
    if [[ "$SHELL_NAME" == "zsh" ]]; then
      PROFILE="$HOME/.zshrc"
      # Prefer .zprofile for login shells if it exists
      [[ -f "$HOME/.zprofile" ]] && PROFILE="$HOME/.zprofile"
    elif [[ "$SHELL_NAME" == "bash" ]]; then
      if [[ "$OS" == "macOS" ]]; then
        PROFILE=${HOME}/.bash_profile
      else
        PROFILE=${HOME}/.bashrc
      fi
    fi
    # Fallback
    [[ -z "$PROFILE" ]] && PROFILE="$HOME/.profile"

    step "Adding $DEST to PATH via $(basename "$PROFILE")"
    mkdir -p "$(dirname "$PROFILE")" || true
    if [[ -w "$(dirname "$PROFILE")" ]] || [[ ! -e "$PROFILE" ]]; then
      # Append only if not already present
      if ! grep -Fq "$DEST" "$PROFILE" 2>/dev/null; then
        printf "\n# Added by Bloch installer\nexport PATH=\"%s:\$PATH\"\n" "$DEST" >> "$PROFILE" 2>/dev/null || {
          warn "Could not write to $(basename "$PROFILE")."
          echo "Add to your shell profile manually: export PATH=\"$DEST:\$PATH\"" >&2
        }
        success "Updated $(basename "$PROFILE"): added $DEST to PATH"
        # Best-effort: export into this process for immediate use
        export PATH="$DEST:$PATH"
        # If running bash and profile is bash-compatible, source it for this subshell
        if [[ "$SHELL_NAME" == "bash" ]] && [[ "$(basename "$PROFILE")" =~ ^(\.bashrc|\.bash_profile|\.profile)$ ]]; then
          step "Refreshing current shell environment"
          # shellcheck disable=SC1090
          . "$PROFILE" 2>/dev/null || true
          hash -r 2>/dev/null || true
        fi
        # Advise user how to refresh their parent shell
        if [[ "$SHELL_NAME" == "zsh" ]]; then
          note "Open a new terminal or run: exec zsh -l (or 'rehash')"
        else
          note "Open a new terminal or run: exec $SHELL -l"
        fi
      else
        note "PATH already contains $DEST in $(basename "$PROFILE")"
      fi
    else
      warn "No write permission for $(dirname "$PROFILE")."
      echo "Add to your shell profile manually: export PATH=\"$DEST:\$PATH\"" >&2
    fi
  fi
fi
