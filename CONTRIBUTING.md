# Contributing to Bloch

Thanks for helping shape Bloch! This guide keeps the contribution process simple while the language evolves rapidly.

---

## Before You Start
- We work from the `develop` branch; create feature branches from there and target it for pull requests. `main` is release-only and always matches the latest tag—no direct commits.
- Please open or reference a GitHub issue so we keep discussion and tracking in one place.
- CI enforces formatting, testing, and Conventional Commit messages, so local checks save time.

---

## Environment Setup

### Prerequisites
- C++20-capable compiler (GCC ≥ 10, Clang ≥ 10, or MSVC 19.28+)
- CMake ≥ 3.16
- Git

### Build & Test

#### Linux & macOS
```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Need debug symbols? Re-run the configure step with `-DCMAKE_BUILD_TYPE=Debug`. The `bloch` binary lives in `build/bin/`; you can execute examples directly with `./build/bin/bloch examples/02_bell_state.bloch`.

#### Windows (Visual Studio)
```powershell
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build --config Release --output-on-failure
```

These commands generate `build/Bloch.sln`, which you can open in Visual Studio for editing and debugging. Executables land under `build/bin/Release/` (or `build/bin/Debug/` when using `--config Debug`).

---

## Workflow Guidelines

### Branches
Use `type/issue-number` (such as `feat/142` or `fix/275`). If you are experimenting, feel free to work on a draft branch locally and rename it before opening a PR.

### Commits & PR Titles
CI expects Conventional Commits:

```
<type>(optional-scope): <summary>
```

Examples: `feat: add runtime cache`, `fix(parser): handle trailing commas`. PR titles should mirror the final squash commit to keep history tidy. Reference related issues in the PR body if needed.

### Pull Requests
1. Push your branch and open a PR against `develop`.
2. Fill in the PR template so reviewers know what changed and how you validated it.
3. Run the basic checks locally (`clang-format`, build, unit tests). Our GitHub workflows rerun them plus CodeQL analysis.
4. We use squash-merge to keep history tidy. The squash commit should match the Conventional Commit format above.

---

## Feature Flags

- Work for future features often lands behind feature flags. Review the process on the [Feature Flags wiki page](https://github.com/bloch-labs/bloch/wiki/Feature-Flags) before starting a branch.
- When developing behind a flag, enable it during configuration (`cmake -S . -B build -DBLOCH_FEATURE_<FLAG>=ON`) and ensure the runtime helpers report the expected state.
- Update the documentation entries whenever you introduce or retire a flag.

---

## Releases
- `main` holds only tagged releases; the default branch is `develop`.
- Release flow:
  1. Merge feature PRs into `develop` (squash with Conventional Commit titles).
  2. release-please auto-opens/updates a release PR from `develop` → `main`.
  3. Review and merge the release PR; release-please tags and creates the GitHub Release on `main`.
  4. Run the **Manual Packager** workflow (Actions → “Manual Packager”) with the new tag to publish artifacts.
  5. Fast-forward `develop` with `main` (`git checkout develop && git merge --ff-only origin/main`) so version bumps stay in sync.
- Hotfixes: branch from `main`, apply the fix, PR back into `main`, merge and tag, then cherry-pick the fix + release commit into `develop` to keep parity.

---

## Code Style & Tooling
- `.clang-format` defines the canonical style—run `clang-format -i <files>` or let your editor handle it.
- Prefer `snake_case` filenames, `PascalCase` types, and `camelCase` functions/methods.
- Tests live in `tests/`; add or update coverage when changing behaviour.

---

## Getting Help
- Open an issue on GitHub for bugs or feature discussions.
- Reach out at [hello@bloch-labs.com](mailto:hello@bloch-labs.com) if you need maintainer feedback or want to coordinate on larger work.

---

