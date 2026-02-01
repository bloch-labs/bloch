# Getting Started

This quick guide helps you install Bloch and run your first program.

## Install

Linux/macOS:
```
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- latest
```

Windows (PowerShell):
```
powershell -NoProfile -ExecutionPolicy Bypass -Command "iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\\bloch-install.ps1; & $env:TEMP\\bloch-install.ps1 -Version latest"
```

From source:
```
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Verify the install:
```
bloch --version
```

## Hello, Bloch
Create `hello.bloch`:
```
function main() -> void {
  echo("Hello, Bloch!");
}
```
Run it:
```
bloch hello.bloch
```

## First quantum program
```
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
```
bloch --emit-qasm flip.bloch
```

## Multi-shot runs and tracking
Annotate qubits with `@tracked` and run multiple shots to aggregate outcomes.

```
function main() -> void {
  @tracked qubit q;
  h(q);
  measure q;
}
```

Run 100 shots:
```
bloch --shots=100 tracked.bloch
```

You can also use `@shots(100)` on `main()` if you prefer to keep the shot count in code.

## Next steps
- Read the [Language Tour](./language-tour.md).
- Use the [Language Specification](./language-spec.md) as the authoritative reference.
- Explore [Examples](./examples.md).
