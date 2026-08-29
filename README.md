> [!IMPORTANT]
> **This repository contains the archived original implementation of the Bloch programming language and is no longer under active development.**
>
> The original `v1.0.x` release line is preserved here as part of Bloch's development and research history. Bloch `v1.0.3` is the software release associated with the 2026 Journal of Open Source Software publication and is permanently archived as Zenodo DOI `10.5281/zenodo.18407424`.
>
> Active development of Bloch restarted in August 2026 in [`bloch-labs/bloch-lang`](https://github.com/bloch-labs/bloch-lang), using a pre-1.0 versioning policy while the language architecture, package model and compiler interfaces are stabilised.
>
> JOSS paper DOI: `10.21105/joss.09625`
>
> ---

# Bloch

Bloch is an open-source, strongly typed, hybrid quantum programming language. It’s designed for people who want classical and quantum code side by side. The toolchain is hardware-agnostic so you can move between simulators and future backends.

## 🚀 Install 
- Linux/macOS:
```
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- latest
```
Swap `latest` for a specific tag (eg `v1.1.0`). The script downloads the matching archive for your platform, verifies its checksum, installs the `bloch` binary into a writable directory, installs the bundled stdlib to your user data directory (for immediate `bloch.lang` usage), and updates your shell profile if needed. Add `INSTALL_DIR=/custom/path` before `bash` to override the binary destination.

#### Windows
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -Command "iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\\bloch-install.ps1; & $env:TEMP\\bloch-install.ps1 -Version latest"
```

Swap `-Version latest` for any tag you want. The script downloads the Windows ZIP, verifies its checksum, installs `bloch.exe` into `%LOCALAPPDATA%\Programs\Bloch` by default, installs stdlib files to `%LOCALAPPDATA%\Bloch\library`, and adds the binary directory to your user `PATH` (opens in new shells).
You can self-update later with `bloch --update`; major version jumps prompt for confirmation and show the changelog link.

- From source: See `CONTRIBUTING.md`

## 💻 VS Code Extension 
Syntax highlighting: https://github.com/bloch-labs/bloch-vscode

## 📚 Documentation 
- User docs: `docs/README.md`
- Contributor docs: `engineering/reference/README.md`

## 📝 Cite 
Use `CITATION.cff` (GitHub’s “Cite this repository” button) for BibTeX/metadata.

## 🤝 Contributing 
Thanks for your interest in contributing! Check the issues tab and read `CONTRIBUTING.md` plus the contributor docs before raising a PR.

## 🌐 Community
You can follow along our journey via the following community links:
- Website: https://bloch-labs.com
- Substack: https://blochlabs.substack.com/subscribe
- X/Twitter: https://x.com/blochlabs
- Email: hello@bloch-labs.com

## 📄 Licence 
Apache 2.0 (see `LICENSE`).
