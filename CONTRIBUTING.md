# Contributing to Bloch

Thank you for your interest in contributing to Bloch! 🎉

We’re building an exciting quantum programming language, and contributions from the community are crucial to its success.

## How to Contribute

This guide explains how to propose changes, which branches to target, and how our CI/CD and release flow works.

## TL;DR
- Open PRs against the correct **release branch** (not `master`) e.g. `release/1.0.x`.
- The correct release branch is determined by the milestone on the issue
- Maintainers will cut release candidates (RCs) from the release branchs; final tags live on `master`.

## Branches

- **`master`**  
  Stable, **released** code only. Every commit here corresponds to a published tag (e.g. `v1.0.3`, `v1.1.0`).

- **`release/1.0.x`**  
  Development and stabilisation for [v1.0.x](https://github.com/bloch-labs/bloch/milestone/5).

- **`release/1.1.x`**  
  Development and stabilisation for [v1.1.x](https://github.com/bloch-labs/bloch/milestone/6).

- **`feature/xx`, `bugfix/xx`, `refactor/xx`, `docs/xx`, `tests/xx`**  
  Short-lived branches for your work, these follow the convention *issue-type/issue-number*.

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
git checkout release/*        # choose appropriate release branch
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
> Do not target `master` with your PR, only RC tags and hotfixes (maintainers) are merged directly to `master`

### PR Reviews
- We’ll review your PR as quickly as possible.
- Please be open to feedback and changes.

### Versioning & releases (how your PR ships)
- Maintainers create RC tags (e.g. `v1.0.3-rc.1`) on `release/*` branches to trigger pre-release builds.
- When an RC is good, maintainers merge the release branch into `master` and create a final tag (e.g. `v1.0.3`). Official binaries are published from that tag.

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