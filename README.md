# Bloch — A Modern Open-Source Quantum Programming Language
> ⚠️ **Active development:** APIs, syntax, and tooling are still evolving and subject to change.

Bloch is a modern, strongly typed, interpreted quantum programming language from [Bloch Labs](https://bloch-labs.com). Paired with our upcoming **Bloch Studio** web IDE and circuit builder, it aims to *unify the quantum development stack* by offering a clean, language‑first, and vendor‑agnostic workflow that scales from research experiments to production circuits. By open-sourcing the core language, we invite the community to help build a sustainable, hardware-agnostic foundation for tomorrow’s quantum applications.

## Demo
![Demo of `/examples/superposition2.bloch`](demo/superposition2-demo.gif)

## Docs
> Official docs are coming soon!
- [Official VSCode extension](https://github.com/bloch-labs/bloch-vscode)

## Installation

Quick install (Linux/macOS):

```bash
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- v1.0.0-rc.1
```

Notes:
- This downloads the `bloch` binary for your OS/arch from the GitHub release `v1.0.0-rc.1`, verifies its checksum, and installs it to `/usr/local/bin` (falls back to `~/.local/bin` if not writable).
- To install a different version, pass the tag (e.g., `v1.0.0`). You can also use `latest` (stable) or `latest-rc` (requires `jq`).
- To choose a custom install directory:

  ```bash
  INSTALL_DIR=$HOME/.local/bin curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- v1.0.0-rc.1
  ```

Manual install:
- Download the asset matching your platform from the release page and extract the tarball: `bloch-<TAG>-<OS>-<ARCH>.tar.gz`
- Move the `bloch` binary to a directory on your `PATH` (e.g., `/usr/local/bin`).

For instructions on how to build from source, see "Building Locally" in [CONTRIBUTING.md](CONTRIBUTING.md). You can also install locally via CMake after building:

```bash
cmake --install build --prefix ~/.local
```

To verify your installation:

```bash
bloch --version
```

## Contributing
Bloch is an open-source project and we welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines and local build instructions.

## Community & Updates

- Website & blog: [bloch-labs.com](https://bloch-labs.com)
- X/Twitter: [@blochlabs](https://x.com/blochlabs) · [@bloch_akshay](https://x.com/bloch_akshay)
- Contact: [hello@bloch-labs.com](mailto:hello@bloch-labs.com)

## License
Bloch is licensed under the [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0) license.
