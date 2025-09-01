#!/usr/bin/env bash
set -euo pipefail

# Usage: ./tag.sh <branch> <tag>
# Example: ./tag.sh release-v1.0.0 v1.0.0-rc.1

if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <branch> <tag>" >&2
  exit 1
fi

BRANCH="$1"
TAG="$2"
MSG=${3:-"Release candidate ${TAG}"}

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "Error: not inside a git repository" >&2
  exit 1
fi

# Ensure we have all refs
git fetch origin --tags --prune || true

# Create local branch tracking remote if needed
if git show-ref --verify --quiet "refs/heads/${BRANCH}"; then
  git checkout "${BRANCH}"
else
  if git ls-remote --exit-code --heads origin "${BRANCH}" >/dev/null 2>&1; then
    git checkout -B "${BRANCH}" "origin/${BRANCH}"
  else
    echo "Error: branch '${BRANCH}' not found locally or on origin" >&2
    exit 1
  fi
fi

# Update branch to latest
git pull --ff-only

# Ensure clean working tree
if ! git diff --quiet || ! git diff --cached --quiet; then
  echo "Error: working tree not clean. Commit/stash changes before tagging." >&2
  exit 1
fi

# Prevent duplicate tag
if git show-ref --tags --verify --quiet "refs/tags/${TAG}"; then
  echo "Error: tag '${TAG}' already exists locally." >&2
  exit 1
fi
if git ls-remote --exit-code --tags origin "refs/tags/${TAG}" >/dev/null 2>&1; then
  echo "Error: tag '${TAG}' already exists on origin." >&2
  exit 1
fi

echo "Creating annotated tag '${TAG}' on $(git rev-parse --short HEAD) (branch ${BRANCH})"
git tag -a "${TAG}" -m "${MSG}"
git push origin "${TAG}"

echo
echo "Pushed tag '${TAG}'."
echo "If '${BRANCH}' matches release-v*, the RC pre-release workflow will run automatically."
echo "View Actions: https://github.com/$(git config --get remote.origin.url | sed -E 's#(git@|https://)github.com[:/](.*)\.git#\2#')/actions"

