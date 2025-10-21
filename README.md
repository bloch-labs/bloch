# Bloch - Modern Quantum Programming

Bloch is a strongly typed, interpreted quantum programming language from [Bloch Labs](https://bloch-labs.com). The language is designed to feel familiar to systems developers while staying hardware-agnostic, so you can move from research sketches to production-grade circuits without switching stacks.

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
We are currently distributing Bloch from source while we stabilise the toolchain. The commands below assume the prerequisites in [CONTRIBUTING.md](CONTRIBUTING.md) are installed.

### Linux & macOS
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

## Project Layout
- `src` — core language implementation.
- `examples` — executable Bloch snippets, including the Bell state demo above.
- `tests` — unit and integration tests covering the compiler and runtime.
- `scripts` — helper scripts used from CI and local development.

## User Documentation
[docs.bloch-labs.com/bloch/overview](https://docs.bloch-labs.com/bloch/overview)

## Developer Documenation
[Wiki](https://github.com/bloch-labs/bloch/wiki/)

## VS Code Extension
[bloch-labs/bloch-vscode](https://github.com/bloch-labs/bloch-vscode)


## Contributing
Contributions are welcome! [CONTRIBUTING.md](CONTRIBUTING.md) covers the development environment, conventions, and the lightweight review process we follow while the language is in active development.

## Community
- Website & blog: [bloch-labs.com](https://bloch-labs.com)
- X/Twitter: [@blochlabs](https://x.com/blochlabs) · [@bloch_akshay](https://x.com/bloch_akshay)
- Contact: [hello@bloch-labs.com](mailto:hello@bloch-labs.com)

## License
Bloch is licensed under the [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0) license.
