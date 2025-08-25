# Bloch — A Modern Open-Source Quantum Programming Language
> ⚠️ **Active development:** APIs, syntax, and tooling are still evolving and subject to change.

Bloch is a modern, strongly typed, interpreted quantum programming language from [Bloch Labs](https://bloch-labs.com). Paired with our upcoming **Bloch Studio** web IDE and circuit builder, it aims to *unify the quantum development stack* by offering a clean, language‑first, and vendor‑agnostic workflow that scales from research experiments to production circuits. By open-sourcing the core language, we invite the community to help build a sustainable, hardware-agnostic foundation for tomorrow’s quantum applications.

## Demo
![Demo of `/examples/superposition2.bloch`](demo/superposition2-demo.gif)

## Docs
> ⚠️ **Note:** docs are still under construction.

See `/docs` for
- [Language grammar](docs/grammar.md)
- Standard library
- [Runtime internals](docs/compiler.md)
- Explore [examples](examples/README.md)
- [Official VSCode extension](https://github.com/bloch-labs/bloch-vscode)

## Quick Start and Contributing

### Prerequisites

- C++17-compatible compiler (e.g. GCC, Clang)
- CMake ≥ 3.15
- Git

### Clone and Build

Clone and build the repo:

```bash
git clone https://github.com/bloch-lang/bloch.git
cd bloch
mkdir build
cd build
cmake ..
make
```
### Run Tests
```bash
cd build
cmake ..
make
ctest
```

### Contributing
Bloch is an open-source project and we welcome contributions! Please see [CONTRIBUTING.md](https://github.com/bloch-lang/bloch/blob/master/CONTRIBUTING.md) for guidelines on how to get involved.

## Community & Updates

- Website & blog: [bloch-labs.com](https://bloch-labs.com)
- X/Twitter: [@blochlabs](https://x.com/blochlabs) · [@bloch_akshay](https://x.com/bloch_akshay)
- Contact: [hello@bloch-labs.com](mailto:hello@bloch-labs.com)

## License
Bloch is licensed under the [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0) license. This permissive license, complete with an explicit patent grant, encourages broad adoption while inviting contributions back to the project.
