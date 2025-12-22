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
- Place `@tracked` **before** a `qubit` or `qubit[]` variable declaration to auto-print counts and probabilities for those qubits.
- Outputs are simulated pseudo-random samples from the statevector; with many shots they converge to theory.

## Files
- `01_hadamard.bloch` — Hadamard on |0⟩ → ~50/50 outcome of 0 and 1  
- `02_bell_state.bloch` — Bell pair ((|00⟩+|11⟩)/√2) → ~50/50 `00` and `11`  
- `03_deutsch_jozsa.bloch` — Deutsch-Jozsa algorithm comparing constant vs balanced oracles  
- `04_grover_search.bloch` — Grover search over 2 qubits, marking `11` as the winner
- `05_teleport_class.bloch` — class-based single-qubit teleportation (entangle + corrections)
- `06_maxcut_c4_class.bloch` — class-based QAOA p=1 for MaxCut on a 4-node cycle

## 01 Hadamard on |0⟩

**File:** `examples/01_hadamard.bloch`

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

**Try:**
```bash
bloch examples/02_bell_state.bloch --shots=1024
```

**Expected output:**
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

## 03 Deutsch-Jozsa (constant vs balanced)

**File:** `examples/03_deutsch_jozsa.bloch`

**Try:**
```bash
bloch examples/03_deutsch_jozsa.bloch
```

**Expected output:**
```
[WARNING]: Qubit q was left unmeasured. No classical value will be returned.
[WARNING]: Qubit q was left unmeasured. No classical value will be returned.
Deutsch-Jozsa constant oracle measurement:
0
Deutsch-Jozsa balanced oracle measurement:
1
```

## 04 Grover search (N = 4)

**File:** `examples/04_grover_search.bloch`

**Try:**
```bash
bloch examples/04_grover_search.bloch --shots=1024
```

**Expected output:**
```
[INFO]: suppressing echo; to view them use --echo=all
Shots: 1024
Backend: Bloch Ideal Simulator
Elapsed: 0.147s

qubit[] data
outcome | count |  prob
--------+-------+-----
11      |  1024 | 1.000


```

## 05 Teleportation (class-based)

**File:** `examples/09_teleport_class.bloch`

**Try:**
```bash
bloch examples/09_teleport_class.bloch --shots=512
```

Prepares a |+> state to teleport, builds a Bell pair via the `Teleporter` class, measures the sender, and applies classical corrections to the receiver.  
**Expected output (representative single-shot echo):**
```
m0=1, m1=1, teleported=0
```
Across many shots, the receiver’s outcome matches the input state (here |+>, so 0/1 ~50/50).

## 06 MaxCut on C4 (class-based QAOA p=1)

**File:** `examples/10_maxcut_c4_class.bloch`

**Try:**
```bash
bloch examples/10_maxcut_c4_class.bloch --shots=1024
```

Encapsulates a p=1 QAOA for the 4-node cycle. Constructor captures `(gamma, beta)`; `run` builds |+>^4, applies the cost on edges (0,1), (1,2), (2,3), (3,0), then a mixer.  
**Expected output:** Histogram concentrated on bitstrings with alternating bits (e.g., `0101`, `1010`), which are the MaxCut optima for C4.

## FAQ

**Do I need a `main()` function?**  
Yes. The Bloch runtime looks up a function literally named `main` and uses it as the entry point. Any other functions you define have to be called (directly or indirectly) from `main`, otherwise they are never executed. `@tracked` counts are collected when a scope ends, so without running `main` the tracked qubits never leave scope and nothing is printed.

**What does `@tracked` do?**  
It aggregates measurement outcomes for the annotated qubit array across shots and prints counts/probabilities.

**Can I mix classical and quantum?**  
Yes! `bit` captures measurement results and you can use normal strongly typed variables (`char`, `string`, `int`, `float`) and control (`if`, `else`, `while`, `for`).
