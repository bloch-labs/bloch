# Bloch Examples

This folder contains small example Bloch programs designed to teach syntax and help you get started with algorithm creation in Bloch.

## How to run

Bloch executes a single `.bloch` file at a time.

```bash
# Single run (default is 1 shot)
bloch examples/01_hadamard.bloch

# Multi-shot run (aggregates results across shots)
bloch examples/01_hadamard.bloch --shots=1024
```

**Notes**
- `--shots=N` repeats the program N times and aggregates tracked measurement outcomes.
- Place `@tracked` **before** a `qubit` or `qubit[]` declaration to auto-print counts and probabilities for those qubits.
- Outputs are simulated pseudo-random samples from the statevector; with many shots they converge to theory.

## Files

- `01_hadamard.bloch` — Hadamard on |0⟩ → ~50/50 outcome of 0 and 1  
- `02_bell_state.bloch` — Bell pair ((|00⟩+|11⟩)/√2) → ~50/50 `00` and `11`

## 01 Hadamard on |0⟩

**File:** `examples/01_hadamard.bloch`
```bloch
function main() -> void {
    @tracked qubit q;

    h(q);
    bit b = measure q;
    echo(b);
}
```

**Try:**
```bash
bloch examples/01_hadamard.bloch --shots=1024
```

**Expected output:**
```
[INFO]: suppressing echo; to view them use --echo=all
Shots: 1024
Backend: Bloch Ideal Simulator
Elapsed: 0.003s

qubit q
outcome | count |  prob
--------+-------+-----
0       |   502 | 0.490
1       |   522 | 0.510
```

## 02 Bell state ((|00⟩+|11⟩)/√2)

**File:** `examples/02_bell_state.bloch`
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

**Try:**
```bash
bloch examples/02_bell_state.bloch --shots=1024
```

**Expected output (typical):**
```
[INFO]: suppressing echo; to view them use --echo=all
Shots: 1024
Backend: Bloch Ideal Simulator
Elapsed: 0.010s

qubit[] qreg
outcome | count |  prob
--------+-------+-----
00      |   506 | 0.494
11      |   518 | 0.506
```

## FAQ

**Do I need a `main()` function?**  
Yes. The Bloch runtime looks up a function literally named `main` and uses it as the entry point. Any other functions you define have to be called (directly or indirectly) from `main`, otherwise they are never executed. `@tracked` counts are collected when a scope ends, so without running `main` the tracked qubits never leave scope and nothing is printed.

**What does `@tracked` do?**  
It aggregates measurement outcomes for the annotated qubit array across shots and prints counts/probabilities.

**Can I mix classical and quantum?**  
Yes! `bit` captures measurement results and you can use normal strongly typed variables (`char`, `string`, `int`, `float`) and control (`if`, `else`, `while`, `for`).
