# Bloch — A Modern Open-Source Quantum Programming Language
> ⚠️ **Active development:** APIs, syntax, and tooling are still evolving and subject to change.

Bloch is a modern, strongly typed, interpreted quantum programming language from [Bloch Labs](https://bloch-labs.com). Paired with our upcoming **Bloch Studio** web IDE and circuit builder, it aims to *unify the quantum development stack* by offering a clean, language‑first, and vendor‑agnostic workflow that scales from research experiments to production circuits. By open-sourcing the core language, we invite the community to help build a sustainable, hardware-agnostic foundation for tomorrow’s quantum applications.

## Demo
![Demo of `/examples/superposition2.bloch`](demo/superposition2-demo.gif)

## Docs
> Official docs are coming soon!
- [Official VSCode extension](https://github.com/bloch-labs/bloch-vscode)

## Installation

Prebuilt binaries for Linux, macOS, and Windows are attached to each release on GitHub.

### Quick Install (from release archive)

- Linux/macOS:
  - Download the `bloch-<tag>-<OS>-<ARCH>.tar.gz` from Releases
  - `tar -xzf bloch-<tag>-<OS>-<ARCH>.tar.gz`
  - `chmod +x install.sh && ./install.sh` (uses `/usr/local/bin` or falls back to `~/.local/bin`)

- Windows:
  - Download the `bloch-<tag>-Windows-<ARCH>.zip` from Releases
  - Extract it and run `install.ps1` in PowerShell (adds Bloch to your user PATH)

Verify installation:

```bash
bloch --version
```

Note: If you prefer to build from source (e.g., for development), see Building Locally in [CONTRIBUTING.md](CONTRIBUTING.md). You can also install locally via CMake after building:

```bash
cmake --install build --prefix ~/.local
```

### Package Managers (planned)

Install via common package managers (enabled for official releases):

- Homebrew (macOS/Linux):
  - `brew tap bloch-labs/bloch`
  - `brew install bloch`

- Windows (Scoop):
  - `scoop bucket add bloch https://github.com/bloch-labs/scoop-bucket`
  - `scoop install bloch`

- Snap (Linux):
  - `sudo snap install bloch --classic`

- AppImage (Linux):
  - Download `bloch-<tag>-Linux-x86_64.AppImage` from the Release
  - `chmod +x bloch-<tag>-Linux-x86_64.AppImage && ./bloch-<tag>-Linux-x86_64.AppImage --appimage-extract-and-run -- --version`

We will continue to ship raw binaries on each GitHub Release for manual installs and CI environments.

## Contributing
Bloch is an open-source project and we welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines and local build instructions.

## Community & Updates

- Website & blog: [bloch-labs.com](https://bloch-labs.com)
- X/Twitter: [@blochlabs](https://x.com/blochlabs) · [@bloch_akshay](https://x.com/bloch_akshay)
- Contact: [hello@bloch-labs.com](mailto:hello@bloch-labs.com)

## License
Bloch is licensed under the [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0) license.
