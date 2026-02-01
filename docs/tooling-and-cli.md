# Tooling & CLI

Run programs with `bloch path/to/file.bloch`.

## Exit codes

- **0** — Success (program ran to completion).
- **Non-zero** — Error (lexical, parse, semantic, or runtime). See [Errors & Diagnostics](./diagnostics.md).

## Common flags

- `--help` — Show usage and available options.
- `--version` — Print the build string and check for updates.
- `--update` — Download and install the latest release.
- `--emit-qasm` — Print the emitted OpenQASM log to stdout (in addition to writing `<input>.qasm` beside your source).
- `--shots=N` — Run the program N times and aggregate tracked values. **Deprecated in v2.0.0;** prefer `@shots(N)` on `main()`. See [Annotations & Tracking](./annotations-and-tracking.md). If both the annotation and this flag are set, the annotation wins and the flag is ignored with a warning.
- `--echo=auto|all|none` — Control echo output. Default `auto` suppresses echo when taking many shots; use `all` for verbose per-shot output, `none` to silence echo.

Each run writes `<input>.qasm` next to your source file. The `--emit-qasm` flag only controls whether that log is also printed to stdout.

## Where is `bloch` installed?

When you install via the install script (Linux/macOS or Windows), the `bloch` binary is placed in a directory on your PATH:

- **Linux/macOS:** The script prefers `~/.local/bin` if it exists and is writable; otherwise `/usr/local/bin` if writable; otherwise `~/.local/bin` (created if needed). You can override with `INSTALL_DIR`, e.g. `INSTALL_DIR=$HOME/.local/bin`.
- **Windows:** See the PowerShell install script output for the install location.

To find the binary: `which bloch` (Linux/macOS) or `where bloch` (Windows).

## Build from source

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The executable is `./build/bin/bloch`. Add it to your PATH or run it with the path when in the repo.

## Editor support

The VS Code extension at [bloch-labs/bloch-vscode](https://github.com/bloch-labs/bloch-vscode) provides syntax highlighting.
