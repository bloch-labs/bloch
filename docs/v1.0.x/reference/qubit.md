---
title: qubit
---

Quantum bit handle managed by the runtime.

## Literals
- None. Qubits must be declared.

## Usage
- Declarations: `qubit q;`, `qubit[2] regs;` (may be `@tracked`).
- Arrays: `qubit[2] regs;`, `qubit[] dyn;` (dynamic size validated at runtime when used).
- Operations: pass to gates (`h(q)`, `cx(q0, q1)`), measure (`bit b = measure q;`), reset (`reset q;`).
- Tracking: add `@tracked` before the declaration to aggregate measurements.

## Errors
- Re-measuring a qubit without `reset` is a runtime error.
- Indexing errors on `qubit[]` are runtime errors.
- `qubit[]` cannot be initialised via literals.
