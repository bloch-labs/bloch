---
title: qubit
---

Handle to a simulator qubit. Declared variables allocate qubits immediately.

### Initialization
- `qubit q;` allocates \(|0\rangle\).
- `@tracked qubit q;` also registers the qubit for histogramming across shots.
- Arrays allocate multiple qubits: `qubit[3] q;`.

### Operations
Apply built-in gates directly to qubit handles. See [Built-ins](../builtins.md) for signatures.

### Measurement and reset
```bloch
bit b = measure q;  // collapses q
reset q;             // returns q to |0\rangle and clears measured flag
```

### Tracking
- Add `@tracked` to collect counts when running with `--shots=N`.
- Untracked qubits simulate normally but do not appear in aggregated results.

### Notes
- Qubits are linear resources in practice; avoid aliasing the same handle across unrelated logic.
- There is no dynamic allocation beyond declaring variables; sizes must be static.
