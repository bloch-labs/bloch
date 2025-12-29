---
title: qubit
---

Handle to a simulator qubit. Qubits are allocated at declaration and manipulated via built-in gates.

### Initialization
- `qubit q;` allocates \(|0\rangle\).
- `@tracked qubit q;` also registers the qubit for histogramming across shots.
- Arrays allocate multiple qubits: `qubit[4] q;`.
- Class fields may be qubits; add `@tracked` to fields you want aggregated across shots.

### Operations
Use built-in gates directly. See [Built-ins](../builtins.md) for signatures.

### Measurement and reset
```bloch
bit b = measure q;  // collapses q
reset q;             // returns q to |0\rangle and clears measured flag
```

### Notes
- Qubits are passed by handle into functions/methods; mutations affect the caller.
- `destroy` affects objects, not qubits. Use `reset` to reuse a qubit slot.
