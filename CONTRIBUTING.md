# Contributing

Thanks for your interest in contributing to Bloch! Please check the issues tab for open work or file a new issue if you spot a bug or have an idea.

## Getting Started

To understand the codebase, start with the contributor docs in `docs/reference/README.md`. When you send a PR, include tests and any necessary doc updates.

## Building from Source
### Linux/MacOS
```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run the compiler against an example with `./build/bin/bloch examples/02_bell_state.bloch --shots=1024`.

### Windows (Visual Studio)
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
  2. release-please runs on `develop`, opens a release PR, and when merged tags + creates the GitHub Release on `develop`.
  3. Fast-forward `main` to the new tag so it stays release-only:
     - `git fetch origin`
     - `git checkout main`
     - `git merge --ff-only <new-tag>`
     - `git push origin main`
  4. Run the **Manual Packager** workflow (Actions → “Manual Packager”) with that tag to publish artifacts.
  5. Fast-forward `develop` with `main` (`git checkout develop && git merge --ff-only origin/main`) if any hotfixes landed directly on `main`.
- Hotfixes:
  1. Branch from `main`, make the fix, and open a PR back to `main`.
  2. After merge, release-please (on `main`) tags and creates the GitHub Release for the patch.
  3. Cherry-pick or fast-forward the hotfix and release commit into `develop` to keep branches aligned.

---

## Code Style & Tooling
- `.clang-format` defines the canonical style—run `clang-format -i <files>` or let your editor handle it.
- Prefer `snake_case` filenames, `PascalCase` types, and `camelCase` functions/methods.
- C++ namespaces mirror `src/bloch/*` (`bloch::compiler`, `bloch::runtime`, `bloch::support`, `bloch::update`); add new code in the matching folder and include it via `bloch/<area>/...`.
- `auto` usage: spell out types when they convey ownership/semantics (AST nodes, unique/shared pointers, optionals, primitives). Use `auto` for iterators, lambdas, obvious temporaries, or verbose templated types; favour readability over minimal typing.
- Tests live in `tests/`; add or update coverage when changing behaviour.

---

## Getting Help
- Open an issue on GitHub for bugs or feature discussions.
- Reach out at [hello@bloch-labs.com](mailto:hello@bloch-labs.com) if you need maintainer feedback or want to coordinate on larger work.

---
