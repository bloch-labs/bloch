# Contributing to Bloch

Thank you for your interest in contributing to Bloch! 🎉

We’re building an exciting quantum programming language, and contributions from the community are crucial to its success.

## 📝 How to Contribute

### 1. Find an Issue

- Check the [Issues](https://github.com/bloch-lang/bloch/issues) tab for open tickets.
- Look for issues labeled `good first issue` or `help wanted`.
- Feel free to open a new issue if you have an idea or have found a bug.

### 2. Branch Naming

Please follow this convention for branches, *issue-type/issue-number*

Examples:
- `feature/02`
- `bugfix/15`
- `refactor/21`
- `docs/27`

This helps us track changes back to issues easily.

### 3. Fork and Clone
```bash
git clone https://github.com/your-username/bloch.git
cd bloch
```

### 4. Create a Branch
```bash
git checkout -b feature/02
```

### 5. Make Changes
- Follow the C++ coding standards used in the repo
- Keep commits focused and descriptive
- Reference the issue number in your commit message
```bash
git commit -m "#15: fix bug" 
```

### 6. Add to the CHANGELOG
- Add an entry to `CHANGELOG.md` describing your change and referencing the issue number eg
```md
#100 - Added 100 new `.bloch` code examples
```

### 7. Push and Create a Pull Request
- Once you've done this please move the issue to the "In Review" column on the project board

### 8. PR Reviews
- We’ll review your PR as quickly as possible.
- Please be open to feedback and changes.

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