---
title: Features
---

Bloch v1.0.x provides the stable foundation of the language and simulator.

## Language fundamentals
- Types: `int`, `float`, `bit`, `string`, `char`, `qubit`, and arrays of each (fixed or dynamic). Arrays may be sized or sized at runtime (validated at runtime).
- Entry point: `function main() -> void` is mandatory.
- `@quantum` marks quantum helper functions; `main` is the entry point.
- `@tracked` on `qubit`/`qubit[]` aggregates measurements at scope exit (printed when you use `--shots`).
- Control flow: `if/else`, `for`, `while`, ternary (`cond ? stmt1 : stmt2`), arithmetic, logical, and bitwise operators.

## Quantum programming
- Built-in gates: `h`, `x`, `y`, `z`, `rx(qubit, theta)`, `ry(qubit, theta)`, `rz(qubit, theta)`, `cx(control, target)`.
- Measure inline: `bit b = measure q;` or inside array literals (`{measure q0, measure q1}`).
- Reset before reuse: `reset q;` returns a qubit to `|0⟩` and clears the measured marker.
- Runtime enforces qubit lifetimes (warns if a qubit leaves scope unmeasured; errors on re-measuring without reset).

## Runtime & execution
- Ideal statevector simulator with OpenQASM emission (`--emit-qasm`).
- Multi-shot execution with aggregated tracked results (`--shots=N`).
- CLI quality-of-life: `--echo=all|none` to control echo suppression; `--update` to fetch newer releases when available.

## Behaviour and diagnostics
- Parse errors for syntax issues (with line/column).
- Semantic errors for type/lifetime violations (e.g., invalid assignment, mismatched array types, re-measuring without reset).
- Runtime errors for division by zero, out-of-bounds, or invalid measurement targets.
- Warnings for unmeasured qubits on scope exit (suppressed during intermediate shots when `--shots` > 1).
