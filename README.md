# Bloch - A Modern Strongly Typed, Hybrid Quantum Programming

Bloch is a strongly typed, interpreted, hybrid quantum programming language from [Bloch Labs](https://bloch-labs.com). The language is designed to feel familiar to systems developers while staying hardware-agnostic, so you can move from research sketches to production-grade circuits without switching stacks.

## Highlights
- Seamless classical/quantum integration with clear type boundaries.
- Readable, expressive syntax that stays approachable under load.
- Hardware-agnostic execution across simulators and future hardware backends.
- Write once, run anywhere as targets evolve.

## See It In Action
![Demo](demo/demo.svg)
Prepare an entangled Bell state, then measure both qubits 8192 times using a tracked array to observe the expected 00 / 11 correlation.

```bloch
@quantum
function createBellState(qubit a, qubit b) -> void {
    h(a);
    cx(a, b);
}

function main() -> void {
    @tracked qubit[2] qreg;

    createBellState(qreg[0], qreg[1]);

    bit[2] out = {measure qreg[0], measure qreg[1]};

    echo(out);
}
```

## Get Started
Bloch is available as source plus manually produced release archives until the automated pipeline lands. The instructions below cover installing pre-built artifacts as well as building from source (the developer workflow described in [CONTRIBUTING.md](CONTRIBUTING.md)).

### Install a release build

#### Linux & macOS
```bash
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/v1.0.0/scripts/install.sh | sh -s -- v1.0.0
```

Change the tag (`v1.0.0`, `v1.0.0-rc1`, etc.) to the artifact you want. The script downloads the correct archive for your platform, verifies its checksum, installs the `bloch` binary into a writable directory, and updates your shell profile if needed. Add `INSTALL_DIR=/custom/path` before `sh` to override the destination.

#### Windows
1. Download the latest `bloch-<tag>-Windows-X64.zip` from the GitHub Releases page.
2. Extract it to a directory such as `%LOCALAPPDATA%\Programs\Bloch`.
3. Add that directory to your `PATH` (Windows Settings → “Edit the system environment variables” → “Environment Variables…” → select `Path` → **Edit** → **New** → paste the folder).
4. Open a new terminal (PowerShell, CMD, or Windows Terminal) and run `bloch.exe --version` to confirm the installation.

### Build from source

#### Linux & macOS
```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run the compiler against an example with `./build/bin/bloch examples/02_bell_state.bloch`.

### Windows (Visual Studio)
```powershell
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build --config Release --output-on-failure
```

This produces a `build/Bloch.sln` you can open directly in Visual Studio for editing and debugging. The `bloch.exe` binary is placed in `build/bin/Release/`.

Additional generator options (Ninja, CLion toolchains, etc.) are covered in [CONTRIBUTING.md](CONTRIBUTING.md).

## User Documentation
[docs.bloch-labs.com/bloch/overview](https://docs.bloch-labs.com/bloch/overview)

## Developer Documenation
[Wiki](https://github.com/bloch-labs/bloch/wiki/)

## Releases
v1.0.0 will ship manually while we build the release pipeline. Run the **Manual Packager** workflow (Actions → “Manual Packager”) against the desired branch/tag to produce Linux, macOS, and Windows archives plus checksums; the GitHub Wiki documents the end-to-end checklist.

## VS Code Extension
[bloch-labs/bloch-vscode](https://github.com/bloch-labs/bloch-vscode)

## Contributing
Contributions are welcome! [CONTRIBUTING.md](CONTRIBUTING.md) covers the development environment, conventions, and the lightweight review process we follow while the language is in active development.

## How to Cite
- If you use Bloch in research, cite it via the repository’s [`CITATION.cff`](CITATION.cff) file (GitHub exposes a “Cite this repository” button) or copy the metadata directly into your BibTeX database. The entry tracks the software release and the forthcoming JOSS article so reviewers can attribute the correct artifact.

## Community
- Website & blog: [bloch-labs.com](https://bloch-labs.com)
- X/Twitter: [@blochlabs](https://x.com/blochlabs) · [@bloch_akshay](https://x.com/bloch_akshay)
- Contact: [hello@bloch-labs.com](mailto:hello@bloch-labs.com)

## License
Bloch is licensed under the [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0) license.
