# Getting Started
**About 5 minutes.**

This quick guide helps you install Bloch and run your first program. By the end you'll have run a classical "Hello, Bloch!" program and a simple quantum program that emits OpenQASM 2.0.

## Prerequisites

- **Supported platforms:** Linux, macOS and Windows.
- **Optional:** [VS Code](https://code.visualstudio.com/) is the recommended IDE with the [Bloch extension](https://github.com/bloch-labs/bloch-vscode) for syntax highlighting.
- **From source only:** [CMake](https://cmake.org/) and a C++17-capable toolchain (e.g. GCC, Clang).

## Install

Linux/macOS:

```bash
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- latest
```

Windows (PowerShell):

```bash
powershell -NoProfile -ExecutionPolicy Bypass -Command "iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\\bloch-install.ps1; & $env:TEMP\\bloch-install.ps1 -Version latest"
```

From source:

```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Verify the install:

```bash
bloch --version
```

## Run your first program

Run a shipped example to confirm everything works. From the repo root (if you have the repo), or using an installed `bloch` with a path to the example file:

```bash
bloch examples/01_hadamard.bloch
```

If you built from source and are in the repo:

```bash
./build/bin/bloch examples/01_hadamard.bloch
```

You should see a single measured bit (0 or 1). For more examples, see [Examples](./examples.md).

## Hello, Bloch

Create `hello.bloch`:

```bloch
function main() -> void {
  echo("Hello, Bloch!");
}
```

Run it:

```bash
bloch hello.bloch
```

## First quantum program

The program below applies a Hadamard gate to one qubit and measures it. **`h(q)`** applies the Hadamard gate, putting the qubit into an equal superposition of \|0⟩ and \|1⟩. **OpenQASM** is a standard intermediate representation for quantum circuits; **`--emit-qasm`** prints the generated circuit to stdout (the runtime also writes `<file>.qasm` next to your source).

```bloch
@quantum
function flip() -> bit {
  qubit q;
  h(q);
  return measure q;
}

function main() -> void {
  bit b = flip();
  echo("Measured: " + b);
}
```

Emit OpenQASM:

```bash
bloch --emit-qasm flip.bloch
```

## Multi-shot runs and tracking

Annotate qubits with `@tracked` and run multiple shots to aggregate outcomes.

```bloch
@shots(100)
function main() -> void {
  @tracked qubit q;
  h(q);
  measure q;
}
```

Run:
```bash
bloch tracked.bloch
```

You can also use the CLI flag `--shots=100` if you prefer (however this will be deprecated in v2.0.0).

## Next steps

- Read the [Language Tour](./language-tour.md).
- Use the [Language Specification](./language-spec.md) as the authoritative reference.
- See [Built-ins and Quantum Gates](./builtins-and-gates.md) for `echo`, gates, and `measure`/`reset`.
- Explore [Examples](./examples.md).
