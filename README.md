# Bloch

Bloch is an open-source, strongly typed hybrid quantum programming language. It’s designed for people who want classical and quantum code side by side. The toolchain is hardware-agnostic so you can move between simulators and future backends.

## 🚀 Install 
- Linux/macOS:
```
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- latest
```
- Windows: 
```
powershell -NoProfile -ExecutionPolicy Bypass -Command "iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\\bloch-install.ps1; & $env:TEMP\\bloch-install.ps1 -Version latest"
```

You can self-update later with `bloch --update`; major version jumps prompt for confirmation and show the changelog link.

- From source: See `CONTRIBUTING.md`

## 💻 VS Code Extension 
Syntax highlighting: https://github.com/bloch-labs/bloch-vscode

## 📚 Documentation 
- User guides: `docs/user/README.md`
- Contributor docs: `docs/reference/README.md`

## 📝 Cite 
Use `CITATION.cff` (GitHub’s “Cite this repository” button) for BibTeX/metadata.

## 🤝 Contributing 
Thanks for your interest in contributing! Check the issues tab and read `CONTRIBUTING.md` plus the reference docs before raising a PR.

## 🌐 Community
You can follow along our journey via the following community links:
- Website: https://bloch-labs.com
- Substack: https://blochlabs.substack.com/subscribe
- X/Twitter: https://x.com/blochlabs
- Email: hello@bloch-labs.com

## 📄 License 
Apache 2.0 (see `LICENSE`).
