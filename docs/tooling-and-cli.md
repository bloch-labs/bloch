# Tooling & CLI

Run programs with `bloch path/to/file.bloch`.

Common flags:
- `--help` shows usage.
- `--version` prints the build string and checks for updates.
- `--update` downloads and installs the latest release.
- `--emit-qasm` prints the emitted OpenQASM log to stdout.
- `--shots=N` runs the program N times and aggregates tracked values (deprecated in v2.0.0;
  prefer `@shots(N)` on `main()`).
- `--echo=auto|all|none` controls echo output (default `auto` suppresses echo when taking many
  shots).

Each run writes `<input>.qasm` beside your source. The `--emit-qasm` flag only controls printing
that log to stdout.

Build from source:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Editor support: the VS Code extension at `https://github.com/bloch-labs/bloch-vscode` provides
syntax highlighting.
