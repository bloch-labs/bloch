# Bloch - A Strongly Typed, Hybrid Quantum Programming

Bloch is an open-source, strongly typed hybrid quantum programming language. The language is designed to be hardware-agnostic, so you can move from research sketches to production-grade circuits without switching stacks.

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
Bloch is available as source plus manually produced release archives until the automated pipeline lands. The instructions below cover installing pre-built artifacts as well as building from source.

### Install a release build

#### Linux & macOS
```bash
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- latest
```

Swap `latest` for a specific tag (eg `v1.0.0`). The script downloads the matching archive for your platform, verifies its checksum, installs the `bloch` binary into a writable directory, and updates your shell profile if needed. Add `INSTALL_DIR=/custom/path` before `bash` to override the destination.
You can also self-update later with `bloch --update`; major version jumps will prompt for confirmation and link the changelog.

#### Windows
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -Command "iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\\bloch-install.ps1; & $env:TEMP\\bloch-install.ps1 -Version latest"
```

Swap `-Version latest` for any tag you want. The script downloads the Windows ZIP, verifies its checksum, installs `bloch.exe` into `%LOCALAPPDATA%\Programs\Bloch` by default, and adds that directory to your user `PATH` (opens in new shells).
You can self-update later with `bloch --update`; major version jumps prompt for confirmation and show the changelog link.

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

## Code Layout
- `bloch::core` (`src/bloch/core/*`): lexer, Pratt parser, AST, and semantic analysis.
- `bloch::runtime` (`src/bloch/runtime/*`): runtime evaluator plus the OpenQASM/statevector simulator.
- `bloch::support` (`src/bloch/support/*`): errors, feature flags, and lightweight helpers shared across layers.
- `bloch::update` (`src/bloch/update/*`): self-update plumbing used by the CLI.

## User Documentation
[docs.bloch-labs.com/bloch/overview](https://docs.bloch-labs.com/bloch/overview)

## VS Code Extension
[bloch-labs/bloch-vscode](https://github.com/bloch-labs/bloch-vscode)

## Contributing
Contributions are welcome! [CONTRIBUTING.md](CONTRIBUTING.md) covers the development environment, conventions, and the lightweight review process we follow while the language is in active development.

## How to Cite
- If you use Bloch in research, cite it via the repository’s [`CITATION.cff`](CITATION.cff) file (GitHub exposes a “Cite this repository” button) or copy the metadata directly into your BibTeX database. The entry tracks the software release and the forthcoming JOSS article so reviewers can attribute the correct artifact.

## Community
- Website: [bloch-labs.com](https://bloch-labs.com)
- Substack: [blochlabs.substack.com](https://blochlabs.substack.com/subscribe)
- X/Twitter: [@blochlabs](https://x.com/blochlabs)
- Contact: [hello@bloch-labs.com](mailto:hello@bloch-labs.com)

## Acknowledgements
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) (MIT) for HTTPS calls used in update checks and downloads.
- [OpenSSL](https://www.openssl.org/) for TLS support during release downloads (prefer statically linked in shipped binaries to avoid runtime deps).

## License
Bloch is licensed under the [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0) license.
