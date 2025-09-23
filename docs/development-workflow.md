# Development Workflow

This guide walks through a typical day-to-day loop for Bloch contributors. It complements `CONTRIBUTING.md` and will eventually be mirrored on the GitHub wiki.

## Branches & Naming

- Base all regular work on `develop`.
- Use `type/ISSUE-NUMBER` naming (`feature/123`, `bugfix/49`, `docs/88`).
- Release branches look like `release-vX.Y.Z` and are created via the "Prepare Release Candidate" workflow.
- Only maintainers create `hotfix/*` branches from `master` for emergency fixes.

## Commit & PR Conventions

- Commits must follow [Conventional Commits](https://www.conventionalcommits.org/) – CI enforces this with commitlint.
- PR titles must be `type: summary (#123)` so the automation can link issues and update the PR body.
- Target `develop` for feature work and the active `release-v*` branch for RC fixes.

## Local Loop

1. Sync `develop`: `git pull origin develop`.
2. Create your branch: `git checkout -b feature/123`.
3. Build and test:
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
   cmake --build build --parallel
   ctest --test-dir build --output-on-failure
   ```
4. Format and lint before pushing: `./scripts/format.sh` (or `--check`).
5. Keep commits focused; reference issues in commit bodies when useful.

## CI Overview

PRs to `develop` or `release-v*`:
- Commitlint, PR title validation.
- Formatting via `clang-format`.
- Debug build + unit tests.
- Coverage enforcement via `gcovr --fail-under-line 90`.

PRs to `master` run the above plus a Release-mode build matrix (Linux/macOS) and packaging dry-run.

## Release Automation Cheatsheet

1. Merge the release-please PR on `develop` (`chore(develop): release vX.Y.Z`).
2. Trigger the **Prepare Release Candidate** workflow (Actions tab) with the target version if you need to override the `VERSION` file.
3. The workflow:
   - Creates/updates `release-vX.Y.Z` from `develop`.
   - Tags `vX.Y.Z-rc.1` which triggers the release-candidate pipeline.
4. Iterate on RCs:
   - Fixes land on `release-vX.Y.Z`.
   - Tag new candidates (`git tag -a vX.Y.Z-rc.N && git push origin vX.Y.Z-rc.N`).
5. When ready, open a PR from `release-vX.Y.Z` to `master`. CI verifies Release builds and packaging.
6. Merging to `master` updates the `VERSION` file and triggers the tagging workflow; the new tag (`vX.Y.Z`) runs the full publish pipeline.

## Cutting Hotfixes

1. Branch from `master`: `git checkout -b hotfix/NNN master`.
2. Apply the fix, update tests, and open a PR to `master`.
3. After merge, cherry-pick or port the fix back to `develop` to keep branches aligned.

## Tooling & Scripts

- `scripts/format.sh` – formatting helper (supports `--check`).
- `scripts/install.sh` – user-facing installer, doubles as smoke test for packaging.
- `scripts/ci/extract_release_notes.py` – used by the release workflow to pull changelog sections.

Questions? Start a discussion or open an issue tagged `question` so we can refine this guide and copy it to the wiki.
