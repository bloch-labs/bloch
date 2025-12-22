# Development Setup

Get a working Bloch development environment for building, testing, and contributing.

## Prerequisites
- C++17 compiler (Clang or GCC on Linux/macOS; MSVC 2019+ on Windows).
- CMake (>= 3.21 recommended) and optionally Ninja for faster builds.
- Python 3.9+ if you plan to edit docs with Poetry.

## Clone and configure
```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

For debug builds:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

With Ninja:
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

## Build
```bash
cmake --build build --parallel
```

The `bloch` binary is placed in `build/bin/`.

## Tests and smoke checks
```bash
ctest --test-dir build --output-on-failure
./build/bin/bloch examples/02_bell_state.bloch
```

## Editor/IDE tips
- Enable CMake or compile_commands.json awareness for navigation and linting.
- Stick to the repository `.clang-format` and follow the coding standards in {doc}`style`.
- On Windows, generate `build/Bloch.sln` with `-G "Visual Studio 17 2022" -A x64` if you prefer the IDE workflow.
