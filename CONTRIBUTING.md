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
git clone https://github.com/bloch-lang/bloch.git
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
- Maintainers will cut release candidates from`develop`; stable and released code tags live on `master`.

## Branches

- **`master`**  
  Stable, **released** code only. Every commit here corresponds to a published tag (e.g. `v1.0.3`, `v1.1.0`).

- **`develop`**
  This is the main development branch. All PRs should be merged here in BAU development

- **`release-v1.0.x`**  
  Release candidates and fixes for [v1.0.x](https://github.com/bloch-labs/bloch/milestone/5).

- **`feature/xx`, `bugfix/xx`, `refactor/xx`, `docs/xx`, `tests/xx`**  
  Short-lived branches for your work, these follow the convention *issue-type/issue-number*.
  These should be merged into `develop `

- **`hotfix/*`** (maintainers)  
  Emergency fixes based off `master`.

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
git commit -m "#15: fix bug" 
```

### Add to the CHANGELOG
- Add an entry to `CHANGELOG.md` describing your change and referencing the issue number eg
```md
#100 - Added 100 new `.bloch` code examples
```

### Open a PR with 
- Clear title and description, reference the issue number in your title e.g. `#15: fixing some bugs`
- Target the correct release branch
> Please make sure to target `develop`, this should be automatic. Do not target `master` with your PR, only RC tags and hotfixes (maintainers) are merged directly to `master`

### PR Reviews
- We’ll review your PR as quickly as possible.
- Please be open to feedback and changes.

### Release & Versioning

Maintainers will:

1) Create a release branch from develop:
   - `git checkout -b release-v1.0.0 origin/develop`

2) Tag the first RC on the release branch and push:
   - `git tag -a v1.0.0-rc.1 -m "RC 1"`
   - `git push origin v1.0.0-rc.1`

   This triggers the pre-release workflow for all OSes and publishes a GitHub pre-release with binaries.

3) Install the RC locally and test:
   - Linux/macOS: download archive and run `./install.sh`
   - Windows: download zip and run `install.ps1`

4) Iterate on the release branch as needed:
   - Commit fixes to `release-v1.0.0` and cut `v1.0.0-rc.2`, `v1.0.0-rc.3`, etc.

5) When stable, merge to master and tag final:
   - `git checkout master && git merge --no-ff release-v1.0.0`
   - `git tag -a v1.0.0 -m "Bloch 1.0.0"`
   - `git push origin master v1.0.0`

Notes:
- RC tags are always `vX.Y.Z-rc.N` created on `release-v*` branches.
- Final tags are `vX.Y.Z` and must point to `master` HEAD.

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
