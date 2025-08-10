# Bloch — A Modern Quantum Programming Language
**Bloch** is a strongly typed, compiled quantum programming language designed for hybrid classical-quantum computation. It combines classical control with OpenQASM-compatible quantum semantics, enabling intuitive yet powerful quantum algorithm development.

Bloch is designed to provide developers with intuitive syntax, robust tooling, and seamless compilation to OpenQASM for execution on real or simulated quantum hardware.

## 📚 Docs
> ⚠ **Note:** docs are still under construction.

See `/docs` for
- [Grammar](docs/grammar.md)
- Standard library
- [How the compiler works](docs/compiler.md)
- [Example programs](examples/README.md)

## 🚀 Usage

> ⚠ **Note:** Bloch is still under active development. Usage instructions will be provided once the compiler reaches a runnable MVP.

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