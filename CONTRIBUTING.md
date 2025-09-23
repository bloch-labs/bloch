# Contributing to Bloch

Thank you for your interest in contributing to Bloch!

## Quick Reference
- Branches come from `develop` (or the active `release-v*` branch for RC fixes).
- Name branches `type/ISSUE-NUMBER` (for example `feature/123`, `bugfix/49`).
- Commits must follow [Conventional Commits](https://www.conventionalcommits.org/) and are linted in CI.
- PR titles must look like `type(optional scope): short summary (#123)` so they link back to an issue.
- CI enforces formatting, build + test success, and **line coverage ≥ 90%**.
- `CHANGELOG.md`, `VERSION`, and release tagging are automated by [release-please](https://github.com/googleapis/release-please).
- See the living documents in [/docs](docs) for architecture notes, workflow deep dives, and release playbooks – these will power the upcoming GitHub wiki.

## Building Locally

### Prerequisites
- C++20-compatible compiler (GCC ≥ 10, Clang ≥ 10, or MSVC 19.28+)
- CMake ≥ 3.16
- Git

### Clone and Build
```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel
```

### Run Tests
```bash
ctest --output-on-failure -C Release
```

## Branch Strategy
- **`master`** – Published and stable releases only. Automation tags every release commit (`vX.Y.Z`).
- **`develop`** – Default branch for day-to-day development. All feature PRs land here.
- **`release-v*`** – Temporary release branches created automatically when a release-please PR is merged. RC fixes target these branches.
- **`feature/*`, `bugfix/*`, `refactor/*`, `docs/*`, `test/*`** – Short-lived topic branches named `type/ISSUE-NUMBER` off `develop` (or the active release branch for RC fixes).
- **`hotfix/*`** – Maintainer-only emergency fixes cut from `master`.

### Branch Naming
```
<type>/<issue-number>  # example: feature/123, bugfix/49
```
Allowed types mirror our commit types: `feat`, `fix`, `bugfix`, `docs`, `style`, `refactor`, `perf`, `test`, `build`, `ci`, `chore`.

## Conventional Commits
Commits are linted with commitlint and must follow the Conventional Commits spec:
```
<type>[optional scope]: <short summary, including issue number>

[optional body]

[optional footer]
```
Use imperative tense for the summary. Reference issues in the footer or body (e.g. `Refs #123`) when useful. Commits such as `feat(parser): add support for ...` or `fix: guard against null handles` will pass linting.

## Working on a Change
1. Make sure an issue exists (open one if needed).
2. Branch from `develop` using the naming convention above.
3. Implement your change, keeping commits focused and lint-friendly.
4. Run `scripts/format.sh` before committing to avoid CI failures.
5. Build and run tests locally (`cmake`, `ctest`) to catch regressions early.

### Pull Request Checklist
- Target `develop` for feature work, or the active `release-v*` branch for RC fixes.
- Keep the PR title in the format `type: summary (#123)` so the automation links the issue.
- Do **not** edit `CHANGELOG.md`, `VERSION`, or `.release-please-manifest.json` manually. Release automation owns these files.
- Expect the PR pipeline to run:
  - Conventional commit linting (all commits in the PR).
  - clang-format verification.
  - Debug build with unit tests and gcovr; fails if total line coverage is below 90%.
  - On PRs targeting `master`, an additional Release build matrix validates packaging across macOS and Linux.
- Release-please runs on `develop` and keeps the changelog up to date; pull requests no longer add entries directly.

## Release Flow
1. **release-please gating** – Every push to `develop` runs `release-please`. When the repo is ready, it opens a PR titled `chore(develop): release vX.Y.Z` that bumps `VERSION` and updates `CHANGELOG.md`.
2. **Merge the release PR** – When that PR is merged into `develop`, automation immediately:
   - Creates `release-vX.Y.Z` from the merge commit.
   - Tags `vX.Y.Z-rc.1`, which triggers the release-candidate workflow.
3. **Release-candidate builds** – Each `vX.Y.Z-rc.N` tag runs a full pre-release build on Linux/macOS that:
   - Verifies formatting, builds, and runs tests in `Release` mode.
   - Packages binaries per platform, generates checksums, and publishes a GitHub pre-release with uploaded artifacts.
4. **Iterating on RCs** – Fix issues directly on `release-vX.Y.Z` and create new tags (`git tag -a vX.Y.Z-rc.2`) to spin another candidate. Every RC tag repeats the pre-release pipeline.
5. **Final release PR** – When the branch is stable, open a PR from `release-vX.Y.Z` to `master`. The PR checks run the usual lint/build/test flow plus a Release-mode build+package matrix to ensure the final bundle is healthy.
6. **Publishing the release** – Merging that PR bumps `master`. Automation tags the merge commit as `vX.Y.Z`, which triggers the final release workflow:
   - Builds and tests Release binaries on Linux and macOS.
   - Packages artifacts, generates checksums, extracts the latest `CHANGELOG` section, and opens a draft GitHub Release with editable notes.
   - Maintainers review/edit the draft release notes and click “Publish release” when satisfied.

For maintainers, [docs/release-process.md](docs/release-process.md) expands each stage with troubleshooting notes and CLI snippets. The GitHub wiki will mirror those details once published.

## ✅ Code Style
- Adhere to the repository `.clang-format` file.
- Recommended VS Code settings:
```json
"editor.formatOnSave": true,
"C_Cpp.clang_format_style": "file",
"[cpp]": {
  "editor.defaultFormatter": "ms-vscode.cpptools"
}
```
- Preferred naming: files `snake_case.cpp`, classes `PascalCase`, methods `camelCase`.
- Use the helper script to format locally:
```bash
./scripts/format.sh          # rewrites files in place
./scripts/format.sh --check  # CI-style verification
```

Thanks again for helping build Bloch! If anything in this guide is unclear, open an issue so we can improve it.
