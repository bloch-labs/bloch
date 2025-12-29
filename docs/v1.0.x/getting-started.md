---
title: Getting Started
---

This guide gets you from zero to a validated Bloch 1.0.x toolchain with two starter circuits. Everything here assumes the 1.0.x surface: functional programs only, no classes, no module imports, no `@shots` annotation.

## Prerequisites
- macOS, Linux, or Windows with a POSIX-like shell for the installer.
- `curl` (for install) and `cmake`/`clang` or MSVC if you prefer to build from source.
- At least 4 GB RAM for the statevector simulator when you scale beyond a few qubits.

## Install Bloch 1.0.2 (release binaries)
Linux / macOS:
```bash
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/v1.0.2/scripts/install.sh | bash -s -- v1.0.2
# Optional: install to a custom dir
# INSTALL_DIR=$HOME/.local/bin bash -s -- v1.0.2
```

Windows (PowerShell):
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -Command "iwr https://raw.githubusercontent.com/bloch-labs/bloch/v1.0.2/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\\bloch-install.ps1; & $env:TEMP\\bloch-install.ps1 -Version v1.0.2"
```

Validate the install and version string:
```bash
bloch --version
```

## Hello, Bloch (|+\rangle)
Create `hello.bloch`:
```bloch
function main() -> void {
    @tracked qubit q;

    h(q);
    bit b = measure q;
    echo(b);
}
```
Run once and emit QASM:
```bash
bloch hello.bloch --emit-qasm
```
The simulator prepares \(\ket{+} = \tfrac{1}{\sqrt{2}}(|0\rangle + |1\rangle)\). With a tracked qubit, repeated shots gather a balanced histogram. Aggregate 1024 shots:
```bash
bloch hello.bloch --shots=1024
```
You should see counts near 512/512 for 0/1 plus a `hello.qasm` file beside the source.

## Bell pair
`bell.bloch`:
```bloch
@quantum
function makeBell(qubit a, qubit b) -> void {
    h(a);
    cx(a, b);
}

function main() -> void {
    @tracked qubit[2] q;
    makeBell(q[0], q[1]);
    bit[2] out = {measure q[0], measure q[1]};
    echo(out);
}
```

The circuit is the standard Bell preparation:
$$
\Qcircuit @C=1em @R=.7em {
  \lstick{|0\rangle} & \gate{H} & \ctrl{1} & \meter \\
  \lstick{|0\rangle} & \qw      & \targ   & \meter \\
}
$$

Run multi-shot to see the expected 00 / 11 correlation:
```bash
bloch bell.bloch --shots=2048
```

## Build from source (optional)
```bash
git checkout v1.0.2
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/bin/bloch examples/02_bell_state.bloch --shots=1024
```

## Troubleshooting
- `bloch: command not found`: ensure your installer target is on `PATH` or re-run the script with a writable `INSTALL_DIR`.
- Syntax errors point to line/column; float literals require a trailing `f` (e.g., `0.5f`).
- Bit literals must be `0b` or `1b`; strings are double-quoted with no escape sequences in 1.0.x.
- When aggregating shots, tracked variables must be annotated with `@tracked` or histograms stay empty.
