---
title: Getting Started
---

Set up Bloch 1.1.x and exercise the new class system, imports, and `@shots` annotation. These steps assume the current `main` branch features (classes, `destroy`, `@shots`, module loader).

## Prerequisites
- macOS, Linux, or Windows with a POSIX-like shell.
- `curl` plus `cmake`/`clang` or MSVC if building from source.
- At least 4 GB RAM for the simulator; more for >20 qubits.

## Install a 1.1.x build
Linux / macOS:
```bash
curl -fsSL https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.sh | bash -s -- latest
# To pin a specific tag: bash -s -- v1.1.0
```

Windows (PowerShell):
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -Command "iwr https://raw.githubusercontent.com/bloch-labs/bloch/HEAD/scripts/install.ps1 -UseBasicParsing -OutFile $env:TEMP\\bloch-install.ps1; & $env:TEMP\\bloch-install.ps1 -Version latest"
```

Verify:
```bash
bloch --version
```

## Hello, Bloch with @shots
Create `hello.bloch`:
```bloch
@shots(1024)
function main() -> void {
    @tracked qubit q;
    h(q);
    bit b = measure q;
    echo(b);
}
```
Run:
```bash
bloch hello.bloch --emit-qasm
```
`@shots` pins the run to 1024 aggregated shots; the CLI `--shots` flag is ignored if it disagrees. The simulator prepares \(\ket{+} = \tfrac{1}{\sqrt{2}}(|0\rangle + |1\rangle)\), so the histogram should be balanced.

## A minimal class example
`teleport.bloch`:
```bloch
class Teleporter {
    public constructor() -> Teleporter = default;

    @quantum
    public function entangle(qubit a, qubit b) -> void {
        h(a); cx(a, b);
    }

    @quantum
    public function teleport(qubit msg, qubit a, qubit b) -> bit[2] {
        entangle(a, b);
        cx(msg, a);
        h(msg);
        bit m0 = measure msg;
        bit m1 = measure a;
        if (m1) { x(b); }
        if (m0) { z(b); }
        return {m0, m1};
    }
}

@shots(2048)
function main() -> void {
    @tracked qubit msg;
    @tracked qubit a;
    @tracked qubit b;

    Teleporter tp = new Teleporter();
    bit[2] m = tp.teleport(msg, a, b);
    bit out = measure b;
    echo("m0=" + m[0] + ", m1=" + m[1] + ", teleported=" + out);
}
```
Run to see a three-qubit teleportation with aggregated counts and a generated `teleport.qasm`.

The circuit core is:
$$
\Qcircuit @C=1em @R=.7em {
  \lstick{|\psi\rangle} & \qw      & \ctrl{1} & \gate{H} & \meter & \cw & \control \cw \\
  \lstick{|0\rangle}    & \gate{H} & \targ    & \qw     & \meter & \cw & \control \cw \\
  \lstick{|0\rangle}    & \qw      & \qw      & \qw     & \qw    & \gate{X^m Z^n} & \meter \\
}
$$

## Build from source (optional)
```bash
git checkout main
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/bin/bloch examples/05_teleport_class.bloch --shots=1024
```

## Troubleshooting
- `@shots` must decorate `main`; other functions will fail semantic checks.
- Float literals require a trailing `f`; bit literals must be `0b` or `1b`.
- When both CLI `--shots` and `@shots` are present, the annotation wins (CLI emits a warning if different).
- Strings use raw characters; escape sequences are not parsed in 1.1.x.
