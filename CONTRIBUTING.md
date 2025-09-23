# Contributing to Bloch

Thank you for your interest in contributing to Bloch! 🎉

We’re building an exciting quantum programming language, and contributions from the community are crucial to its success.

## How to Contribute

This guide explains how to propose changes, which branches to target, and how our CI/CD and release flow works.

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

## TL;DR
- Open PRs against `develop` for BAU feature development.
- If fixing bugs identified in a release candidate (RC) then open a PR against the appropriate `release-v*` branch.
- Release hardening happens on `release-v*`; once the branch stabilises it merges into `master` through a release PR and gets tagged as `vX.Y.Z`.
- Immediately after tagging `master`, fast-forward `develop` so that the new release is the starting point for future work.
- Maintainers cut release candidates from `develop`; stable and released code tags live on `master`.

## Branches
**`master`**  
- Stable, **released** code only. Every commit here corresponds to a published tag (e.g. `v1.0.3`, `v1.1.0`).

**`develop`**
- This is the main development branch. All PRs should be merged here in BAU development.

**`release-v*`**
- Release branches from which RCs are cut and where release bug fixes land.
- Release PRs targeting `master` must originate from these branches.

**`feature/xx`, `bugfix/xx`, `refactor/xx`, `docs/xx`, `tests/xx`**
- Short-lived branches for your work, these follow the convention *issue-type/issue-number*.
- These should be merged into `develop`.

**`hotfix/*`** (maintainers)  
- Emergency fixes based off `master`.

## Raising a PR
### Clone the repo
```bash
git clone https://github.com/your-username/bloch.git
cd bloch
```

### Create a Branch
```bash
git checkout develop          # develop is the default branch
git checkout -b feature/xx    # label with ticket number  
```

### Make Changes
- Follow the C++ coding standards used in the repo
- Keep commits focused and descriptive
- Reference the issue number in your commit message e.g.
```bash
git commit -m "fix: guard against invalid measurement angles (#15)"
```

### Add to the CHANGELOG
- For PRs targeting `develop` or `release-v*`, add an entry to `CHANGELOG.md` describing your change and referencing the issue number e.g.
```md
#100 - Added 100 new `.bloch` code examples
```
- Release PRs going into `master` should already contain the changelog entries that landed on the release branch; you shouldn't need to touch the changelog again at that stage.
- If a change truly does not warrant a changelog entry, add `[no-changelog]` to your PR title, body, or last commit message to silence the check.

### Commit message style (Conventional Commits)
- All commits in a pull request must follow the [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) format so that automation such as release-please can understand the change type.
- Format commits as `type(scope): short summary`. Examples:
  - `feat(parser): add support for controlled operations`
  - `fix: guard against null program handles`
- Allowed types include `build`, `chore`, `ci`, `docs`, `feat`, `fix`, `perf`, `refactor`, `revert`, `style`, `test`, `release`, and `deps`.
- Use `!` after the type/scope or add a `BREAKING CHANGE:` footer when a change is breaking.
- The "PR Checks / Conventional Commits" job will fail if any commit message violates the convention; amend or rebase before requesting review.

### Open a PR with
- Clear title and description, reference the issue number in your title e.g. `#15: fixing some bugs`
- Target the correct release branch
> Please make sure to target `develop` by default. Maintainers will raise the release PR from `release-v*` into `master` when we are ready to ship.

### Required status checks & branch protection

We rely on GitHub *Rulesets* (recommended) or classic branch protection rules to enforce the correct checks on every PR. Configure the following rules in the repository settings:

| Branch pattern | Required status checks | Notes |
| -------------- | ---------------------- | ----- |
| `develop`      | `PR Checks / Conventional Commits`, `PR Checks / Build and Test`, `Check for Changelog Update` | Require pull requests before merging and at least one approval. |
| `release-v*`   | `PR Checks / Conventional Commits`, `PR Checks / Build and Test`, `Check for Changelog Update` | Mirrors `develop` but applies to the release hardening branches. |
| `master`       | `PR Checks / Conventional Commits`, `PR Checks / Build and Test`, `PR Checks / Release packaging dry run` | Enable "Require pull request" and "Require linear history"; only maintainers should merge here. |

When creating the ruleset, make sure that "Require status checks" references the job names exactly as above (GitHub prefixes them with the workflow name, e.g. `PR Checks / Build and Test`). You can optionally enable the same checks as *required* on the matching ruleset so they gate merges automatically.

For repositories using the new ruleset experience: add a **ruleset** named e.g. `develop` matching the `develop` branch and mark the workflow checks above as required. Repeat for `release-v*` (use the pattern in the ruleset) and `master`. For the classic branch protection experience: create three branch protection rules using the same patterns and enable "Require status checks to pass" with the checks listed.

### PR Reviews
- We’ll review your PR as quickly as possible.
- Please be open to feedback and changes.

## Release & Versioning

Maintainers will:

1) Create a release branch from develop:
   - `git checkout -b release-v1.0.0 origin/develop`

2) Tag the first RC on the release branch and push:
   - `git tag -a v1.0.0-rc.1 -m "RC 1"`
   - `git push origin v1.0.0-rc.1`

   This triggers the **Release Candidate Build** workflow for all OSes and publishes a GitHub pre-release with binaries.

3) Install the RC locally and test:

4) Iterate on the release branch as needed:
   - Commit fixes to `release-v1.0.0` and cut `v1.0.0-rc.2`, `v1.0.0-rc.3`, etc.

5) When stable, raise a PR from `release-v1.0.0` to `master`:
   - The PR will run the `PR Checks` workflow, including the release packaging dry run, to prove the release can be built from scratch.
   - Once merged, tag the final release:
     - `git tag -a v1.0.0 -m "Bloch 1.0.0"`
     - `git push origin v1.0.0`
   - Pushing the tag (which must point at `master`) triggers the **Publish Release** workflow that builds artifacts and creates the official GitHub release.

6) Sync `develop` with the released code:
   - `git checkout develop`
   - `git merge --ff-only origin/master`
   - Push the fast-forward so that new features start from the freshly released baseline.

Notes:
- RC tags are always `vX.Y.Z-rc.N` created on `release-v*` branches.
- Final tags are `vX.Y.Z` and must point to `master` HEAD so the publish workflow can run.
- After every release, `develop` should be fast-forwarded to `master` so the next cycle inherits the released code and changelog entries.

## Automating changelog curation with release-please

We ship a [release-please](https://github.com/google-github-actions/release-please-action) workflow to take the busywork out of changelog maintenance:

- The **Release Please** workflow runs on every push to `develop` (and can be started manually) to look at the Conventional Commit history since the last release.
- Update the starting version in `.github/release-please-manifest.json` if the current release train should begin from a different semantic version than `0.1.0`.
- When there are user-facing changes, it opens a draft PR against `develop` titled `chore: prepare release ${version}` that updates `CHANGELOG.md` and bumps the tracked version in `.github/release-please-manifest.json`.
- Review the generated PR, make any manual edits to the changelog, and merge it when you're ready to branch for release. The merge commit becomes the starting point for the `release-vX.Y.Z` branch.
- The workflow is configured with `handle-gh-release: false`, so it will not publish GitHub releases automatically—continue to follow the release steps above for RCs and finals.
- You can trigger the workflow from the Actions tab (`Release Please` > **Run workflow**) or via the CLI: `gh workflow run release-please.yaml`.

## Suggested developer wiki pages

The end-user documentation lives elsewhere, so use the GitHub wiki for internal engineering context. Recommended pages include:

- **Architecture overview** – diagrams of the compiler/runtime pipeline, key modules, and extension points.
- **Build & tooling** – dependency setup per platform, IDE tips, and how to run the CI workflows locally.
- **Release playbook** – a copy of the release checklist from this guide with repo-specific secrets, signing steps, and communication templates.
- **Testing strategy** – how we structure unit/integration/benchmark tests and the expectations for new contributions.
- **Operational cookbook** – procedures for triaging incidents, backporting fixes to `release-v*`, and auditing telemetry (if applicable).

## ✅ Code Style
- Adhere to the `.clang-format` file in the repo
- Set up auto-formatting on file save in your `.vscode/settings.json` file as follows
```json
"editor.formatOnSave": true,
"C_Cpp.clang_format_style": "file",
"[cpp]": {
   "editor.defaultFormatter": "ms-vscode.cpptools"
}
```
- Follow consistent naming conventions:
    - files: `snake_case.cpp`
    - classes: `PascalCase`
    - methods: `camelCase`
- You can format manually by running 
```bash
clang-format -i $(find . -regex '.*\.\(cpp\|hpp\|c\|h\)')
```
