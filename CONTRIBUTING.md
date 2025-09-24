# Contributing to Bloch

Thank you for your interest in contributing to Bloch! We’re building a modern quantum programming language and the community is a core part of that journey.

This guide explains how to work on the project, the conventions enforced by CI, and how code moves from a feature branch into a production release.

---

## TL;DR Flow
- `develop` is the integration branch you branch from and target for day-to-day work.
- Name branches `type/issue-number` (e.g. `feat/142`) using the allowed types below.
- Every commit follows Conventional Commits **and** ends with the issue number: `feat: add runtime cache (#142)`.
- PR titles mirror the commit format and link the same issue (`feat: add runtime cache (#142)`).
- Each PR runs **PR Quality** (format, build, test), **CodeQL static analysis**, and commit linting automatically.
- Once merged to `develop`, the **release-please bot** opens/updates a release PR that maintains the changelog.
- Merging that release PR spins up a `release-vX.Y.Z` branch, tags `vX.Y.Z-rc.1`, and kicks off the release-candidate workflow.
- When the release branch is ready, merge `release-vX.Y.Z` into `master`; this runs a release dry run and, after tagging `vX.Y.Z`, publishes the production release.

---

## Local Development

### Prerequisites
- C++20-compatible compiler (GCC ≥ 10, Clang ≥ 10, or MSVC 19.28+)
- CMake ≥ 3.16
- Git

### Build & Test
```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

---

## Branching & Commit Conventions

### Branch names
Use `type/issue-number`, where `type` is one of:
`feat`, `fix`, `bugfix`, `hotfix`, `chore`, `docs`, `refactor`, `perf`, `test`, `build`, `ci`, `style`, `revert`.

Examples:
- `feat/142`
- `bugfix/275`

Release automation creates branches named `release-vX.Y.Z`; do not use that prefix manually.

### Commit messages
All commits are linted in CI. The enforced pattern is:
```
<type>(optional-scope): <summary> (#<issue>)
```
Examples:
- `feat: add runtime cache (#142)`
- `fix(parser): handle trailing commas (#278)`

Breaking changes can use the Conventional Commits bang notation (`feat!: ...`). Put the issue number that tracks the work in the summary.

Release automation commits (e.g., `chore: release 1.2.0`) are ignored by the linter, everything else must comply.

### PR titles
PR titles follow the same format as commits so that automation can pick up the issue number: `type: useful summary (#142)`.

---

## Pull Requests
1. Branch from `develop`: `git checkout -b feat/142 origin/develop`.
2. Make changes and keep commits focused; stick to the formatting rules (`clang-format` is enforced in CI).
3. Push and open a PR against the correct base:
   - Normal work → base `develop`
   - Release hardening fixes → base `release-vX.Y.Z`
4. Fill in the PR template and ensure it references the issue the PR closes.

### Automated checks on every PR
- **PR Quality (`.github/workflows/pr-checks.yaml`)**
  - Validates branch name & PR title
  - Lints commits (Conventional Commit + issue number)
  - Verifies formatting (`clang-format`), builds, and runs unit tests (Release configuration)
- **Static Analysis (`.github/workflows/static-analysis.yaml`)**
  - Runs CodeQL security + quality analysis for C++ (results appear in the Security tab and as PR annotations)

All jobs must be green before maintainers will review. If the release bot opens a PR, the commit lint step is skipped automatically.

---

## Release Automation
Our CI/CD pipeline automates the full release process end-to-end:

1. **Merge into `develop`:**
   - Triggers `Release Please` workflow which runs `release-please`. It updates `.release-please-manifest.json` and opens/updates a bot-authored release PR (`chore: release X.Y.Z`) that includes the changelog.

2. **Merge the release PR:**
   - `Promote Release` workflow detects the merged release PR, creates `release-vX.Y.Z` from the merge commit, tags `vX.Y.Z-rc.1`, and pushes both.
   - Tagging kicks off `release-candidate.yaml`, which builds on Linux/macOS, uploads artifacts, and publishes a GitHub pre-release (`vX.Y.Z-rc.1`).
   - `develop` is effectively frozen for that version; new work can continue and will accumulate for the next release PR.

3. **Release hardening:**
   - Fixes for the pending release land via PRs targeting `release-vX.Y.Z`. They run the same PR Quality + CodeQL checks. Additional RC tags (e.g., `vX.Y.Z-rc.2`) can be created manually if needed by tagging the release branch; the existing workflow will process them.

4. **Final review:**
   - Open a PR from `release-vX.Y.Z` into `master`. `release-checks.yaml` performs a full Release build/test plus a `cpack` packaging dry run.

5. **Publish:**
   - Merge the PR and tag `master` with `vX.Y.Z`.
   - The `Publish Release` workflow runs on that tag: it rebuilds artifacts for Linux/macOS, generates checksums, extracts release notes from the changelog, and publishes the GitHub release with assets attached.

All automation runs from the repository’s default tokens; no manual changelog edits are needed because release-please owns `CHANGELOG.md`.

---

## Code Style
- `.clang-format` defines the canonical style; CI enforces it.
- Prefer `snake_case` filenames, `PascalCase` classes, and `camelCase` methods.
- Run `clang-format` locally before committing (`clang-format -i <files>` or the VSCode integration).

### CI helper scripts
- `scripts/ci/extract_release_notes.py`: invoked from `.github/workflows/release-publish.yaml` to pull the relevant section from `CHANGELOG.md` when publishing a release.

---

## Questions?
- Open an issue on GitHub
- Reach out to the team at [hello@bloch-labs.com](mailto:hello@bloch-labs.com)

Happy coding!
