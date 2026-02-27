# Built-ins and Quantum Gates

This page is the single reference for built-in functions and quantum operations in Bloch. For full language rules, see the [Language Specification](./language-spec.md).

## Built-in function

| Name | Signature | Description |
|------|-----------|-------------|
| `echo` | `echo(expr)` | Prints a human-readable representation of the expression to stdout. |

`echo` is reserved and may not be redefined.

## Quantum gates

All gates return `void` and operate on qubit(s). Gate names are reserved.

| Gate | Operands | Description |
|------|----------|-------------|
| `h` | `h(qubit)` | Hadamard: creates equal superposition \|0⟩ + \|1⟩. |
| `x` | `x(qubit)` | Pauli-X (bit flip). |
| `y` | `y(qubit)` | Pauli-Y. |
| `z` | `z(qubit)` | Pauli-Z (phase flip). |
| `rx` | `rx(qubit, angle)` | Rotation about X by `angle` (radians, `float`). |
| `ry` | `ry(qubit, angle)` | Rotation about Y by `angle` (radians, `float`). |
| `rz` | `rz(qubit, angle)` | Rotation about Z by `angle` (radians, `float`). |
| `cx` | `cx(control, target)` | Controlled-X (CNOT): applies X to `target` when `control` is \|1⟩. |

## Measure and reset

| Operation | Form | Description |
|-----------|------|-------------|
| `measure` | `measure expr;` (statement) or `measure expr` (expression) | Collapses the qubit(s) to a classical outcome. Statement form has no return; expression form returns a `bit` (single qubit) or is used in expressions for measurement results. Applying a gate to a qubit after measurement without first calling `reset` is a runtime error. |
| `reset` | `reset expr;` | Resets the qubit(s) to \|0⟩. Use after measurement if you need to apply more gates to the same qubit. |

- For a single qubit: `measure q` returns a `bit` (0 or 1).
- For `qubit[]`, you typically measure elements individually (e.g. `measure qreg[0]`) or in sequence and collect bits.
- `reset` accepts a qubit or qubit array; it does not return a value.

## See also

- [Quantum Programming](./quantum-programming.md) — Declaring qubits and writing `@quantum` functions.
- [Annotations & Tracking](./annotations-and-tracking.md) — `@tracked` and multi-shot aggregation.
