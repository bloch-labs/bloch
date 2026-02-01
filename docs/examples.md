# Examples

The repository ships with runnable programs you can tweak. Try these in order: **01 → 02 → 05** for quantum; **07** and **08** for classes and generics.

## Example list

- `examples/01_hadamard.bloch` — Apply H to a single qubit and measure.
- `examples/02_bell_state.bloch` — Prepare an entangled pair and observe correlated outcomes.
- `examples/03_deutsch_jozsa.bloch` — Oracle-based Deutsch–Jozsa sketch.
- `examples/04_grover_search.bloch` — Grover iteration outline.
- `examples/05_teleport_class.bloch` — Teleportation with classical scaffolding.
- `examples/06_maxcut_c4_class.bloch` — MaxCut on 4 nodes.
- `examples/07_null_demo.bloch` — Using `null` with class references.
- `examples/08_generics.bloch` — Generic classes and type bounds.
- `examples/multifile/*` — Imports across modules (see [Multifile](#multifile) below).

## Try these in order

- **Quantum:** 01 → 02 → 05 (Hadamard, Bell state, teleportation).
- **Classes/generics:** 07 → 08 (null and class references, then generics).

## Key examples with code and output

### 01 — Hadamard

Apply the Hadamard gate to one qubit and measure. You get 0 or 1 with equal probability.

```bloch
function main() -> void {
    @tracked qubit q;

    h(q);
    bit b = measure q;
    echo(b);
}
```

**Run (installed):**

```bash
bloch examples/01_hadamard.bloch
```

**Run (from source, repo root):**

```bash
./build/bin/bloch examples/01_hadamard.bloch
```

**Sample output:** A single bit, e.g. `0` or `1`, plus tracked summary if multiple shots are used.

### 02 — Bell state

Create an entangled pair with `h` and `cx`, then measure both qubits. Outcomes are correlated (00 or 11).

```bloch
@quantum
function createBellState(qubit a, qubit b) -> void {
    h(a);
    cx(a, b);
}

function main() -> void {
    @tracked qubit[2] qreg;

    createBellState(qreg[0], qreg[1]);

    bit[2] out = {measure qreg[0], measure qreg[1]};

    echo(out);
}
```

**Run:**

```bash
bloch examples/02_bell_state.bloch
```

Or with QASM printed to stdout:

```bash
bloch examples/02_bell_state.bloch --emit-qasm
```

**Sample output:** A two-bit result such as `[0, 0]` or `[1, 1]` (correlated).

## Multifile

The `examples/multifile/` directory shows imports across modules.

**Layout:**

- `main.bloch` — Entry point; imports `QuantumMath` and calls its static functions.
- `QuantumMath.bloch` — A `static class` with constants and helper functions (e.g. `PI`, `energyFromFrequency`, `phaseAccumulation`).

**Import in `main.bloch`:**

```bloch
import QuantumMath;

function main() -> void {
    float freqHz = 20000000.0f;
    float omega = 2.0f * QuantumMath.PI * freqHz;
    // ...
}
```

**Run:** Pass the file that contains `main()` (the entry point). From the repo root:

```bash
bloch examples/multifile/main.bloch
```

Or with a built-from-source binary:

```bash
./build/bin/bloch examples/multifile/main.bloch
```

The module loader resolves `import QuantumMath;` to `QuantumMath.bloch` in the same directory (or as per your project layout). Exactly one `main()` must exist across all imported modules.

## Building and running from source

If you don’t have `bloch` on your PATH, build then run with the built binary:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/bin/bloch examples/02_bell_state.bloch --emit-qasm
```

Use `--echo=all` for verbose output during multi-shot runs.
