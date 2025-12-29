---
title: Built-ins
---

Built-in gates and utilities available in Bloch 1.0.x.

### Quantum gates
| Name | Signature | Effect |
| --- | --- | --- |
| `h` | `h(qubit)` | Hadamard: \(\tfrac{1}{\sqrt{2}}(|0\rangle+|1\rangle)\) or \(\tfrac{1}{\sqrt{2}}(|0\rangle-|1\rangle)\) depending on input. |
| `x` | `x(qubit)` | Pauli-X. |
| `y` | `y(qubit)` | Pauli-Y. |
| `z` | `z(qubit)` | Pauli-Z. |
| `rx` | `rx(qubit, float radians)` | Rotation about X by `radians`. |
| `ry` | `ry(qubit, float radians)` | Rotation about Y by `radians`. |
| `rz` | `rz(qubit, float radians)` | Rotation about Z by `radians`. |
| `cx` | `cx(qubit control, qubit target)` | Controlled-X. |

### Measurement
- `measure q` returns `bit` and collapses `q`.
- `reset q` returns `q` to \(|0\rangle\).

### Diagnostics
- `echo(value)` prints classical data during execution. Arrays render as comma-separated lists. Suppressed on intermediate shots unless `--echo=all` is used.

### Notes
- Gate names are reserved; user functions cannot shadow them.