# Bloch — A Modern Quantum Programming Language
**Bloch** is a strongly typed, interpreted quantum programming language designed for hybrid classical-quantum computation. It combines classical control with OpenQASM-compatible quantum semantics, enabling intuitive yet powerful quantum algorithm development.

Bloch provides developers with intuitive syntax, robust tooling, and runtime generation of OpenQASM for execution on real or simulated quantum hardware.

## 📚 Docs
> ⚠ **Note:** docs are still under construction.

See `/docs` for
- [Grammar](docs/grammar.md)
- Standard library
- [How the runtime works](docs/compiler.md)
- [Example programs](examples/README.md)

## 🚀 Usage

> ⚠ **Note:** Bloch is still under active development. Usage instructions will be provided once the runtime reaches a runnable MVP.

## 🛠 Developer Instructions

### Prerequisites

- C++17-compatible compiler (e.g. GCC, Clang)
- CMake ≥ 3.15
- Git

### Build

Clone the repo:

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

## 🤝 Contributing
Bloch is an open-source project and we welcome contributions! Please see [CONTRIBUTING.md](https://github.com/bloch-lang/bloch/blob/master/CONTRIBUTING.md) for guidelines on how to get involved.

## 📄 License
This project is open-source under the Apache 2.0 License. See LICENSE for details.

## 🔒 Why Apache 2.0?

We’ve chosen the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0) for Bloch because:

- **Enterprise-friendly**: Permissive license widely trusted by companies, researchers, and investors.
- **Patent protection**: Includes an explicit patent grant, important for a runtime stack in a rapidly evolving field.
- **Ecosystem growth**: Encourages broad adoption by academia and industry, without barriers to entry.
- **Contributions welcome**: While users can adopt freely, improvements to Bloch itself are encouraged via pull requests.

This ensures Bloch remains open, extensible, and widely usable — while Bloch Studio, our SaaS platform, builds on top with commercial features.