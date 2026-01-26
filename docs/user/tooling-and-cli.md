# Tooling & CLI

Run programs with `bloch path/to/file.bloch`. Add `--emit-qasm` to print the generated OpenQASM. Multi-shot runs come from `@shots(N)` on `main()` or the deprecated `--shots=N` flag. Control echo with `--echo=auto|all|none` (the default `auto` suppresses echo when taking many shots).

For versioning and updates, `bloch --version` prints the build string and performs a quick update check, while `bloch --update` downloads and installs the latest release, prompting on major changes.

Each run writes `<input>.qasm` beside your source. To build from source: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`, `cmake --build build --parallel`, and `ctest --test-dir build --output-on-failure`.

For editor support, the VS Code extension at `https://github.com/bloch-labs/bloch-vscode` provides syntax highlighting. Use the programs in `examples/` as quick sanity checks; see `docs/user/examples.md`.
