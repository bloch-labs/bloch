#!/usr/bin/env bash
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
#   curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | sh -s -- v1.0.0-rc.1
#   curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | sh -s -- latest
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

# Destination directory logic
DEST=${INSTALL_DIR:-/usr/local/bin}
if [[ ! -w "$DEST" ]]; then
  # fallback to ~/.local/bin if not writable
  DEST="$HOME/.local/bin"
fi
mkdir -p "$DEST"

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
if [[ -t 1 ]]; then
  curl -fsSL -# "$CHECKSUMS_URL" -o "$TMPDIR/checksums.txt" || {
    warn "checksums.txt not found; skipping verification"
    NO_CHECKSUM=1
  }
else
  curl -fsSL "$CHECKSUMS_URL" -o "$TMPDIR/checksums.txt" || {
    warn "checksums.txt not found; skipping verification"
    NO_CHECKSUM=1
  }
fi

if [[ -z "${NO_CHECKSUM:-}" ]]; then
  step "Verifying checksum"
  if command -v sha256sum >/dev/null 2>&1; then
    EXPECTED=$(grep "  ${ASSET}$" "$TMPDIR/checksums.txt" | awk '{print $1}')
    ACTUAL=$(sha256sum "$TMPDIR/$ASSET" | awk '{print $1}')
  else
    EXPECTED=$(grep "  ${ASSET}$" "$TMPDIR/checksums.txt" | awk '{print $1}')
    ACTUAL=$(shasum -a 256 "$TMPDIR/$ASSET" | awk '{print $1}')
  fi
  if [[ -z "${EXPECTED:-}" ]]; then
    warn "expected checksum not found for ${ASSET}; skipping verification"
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

if [[ ! -f "$TMPDIR/bloch" ]]; then
  error "extracted archive did not contain 'bloch' binary"
  exit 1
fi

chmod +x "$TMPDIR/bloch"
install -m 0755 "$TMPDIR/bloch" "$DEST/bloch"

success "Installed: $DEST/bloch"
note "You can verify with: '$DEST/bloch --version'"

# Offer to add DEST to PATH if not already there
path_has_dest=0
case ":$PATH:" in
  *":$DEST:"*) path_has_dest=1 ;;
esac

if ! command -v bloch >/dev/null 2>&1 || [[ $path_has_dest -eq 0 ]]; then
  if has_tty && [[ -r /dev/tty ]]; then
    SHELL_NAME=$(basename "${SHELL:-}")
    if [[ "$SHELL_NAME" == "fish" ]]; then
      warn "Detected fish shell. Automatic PATH update is not supported."
      echo "Add this line to ~/.config/fish/config.fish:" >&2
      echo "  set -gx PATH \"$DEST\" \$PATH" >&2
    else
      # Choose a profile file
      PROFILE=""
      if [[ "$SHELL_NAME" == "zsh" ]]; then
        PROFILE="$HOME/.zshrc"
      elif [[ "$SHELL_NAME" == "bash" ]]; then
        if [[ "$OS" == "macOS" ]]; then
          PROFILE=${HOME}/.bash_profile
        else
          PROFILE=${HOME}/.bashrc
        fi
      fi
      # Fallbacks
      [[ -z "$PROFILE" ]] && PROFILE="$HOME/.profile"
      [[ -f "$HOME/.zprofile" && "$SHELL_NAME" == "zsh" ]] && PROFILE="$HOME/.zprofile"

      step "'bloch' is not on PATH"
      printf "Would you like to add '%s' to your PATH by updating %s? [y/N] " "$DEST" "$(basename "$PROFILE")" > /dev/tty
      read -r reply < /dev/tty || reply="n"
      if [[ "$reply" == [yY] || "$reply" == "yes" || "$reply" == "YES" ]]; then
        mkdir -p "$(dirname "$PROFILE")"
        # Append only if not already present
        if ! grep -Fq "$DEST" "$PROFILE" 2>/dev/null; then
          printf "\n# Added by Bloch installer\nexport PATH=\"%s:\$PATH\"\n" "$DEST" >> "$PROFILE"
          success "Updated $(basename "$PROFILE"): added $DEST to PATH"
          note "Open a new terminal or run: source $PROFILE"
        else
          note "PATH already contains $DEST in $(basename "$PROFILE")"
        fi
      else
        note "Skipped updating PATH. You can add it manually with:"
        echo "  echo 'export PATH=\"$DEST:\$PATH\"' >> ~/.profile && source ~/.profile"
      fi
    fi
  else
    warn "Non-interactive shell: 'bloch' may not be on PATH."
    echo "Add to your shell profile: export PATH=\"$DEST:\$PATH\"" >&2
  fi
fi
