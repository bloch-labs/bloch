---
title: Built-ins
---

Built-in gates and utilities in Bloch 1.1.x.

### Quantum gates
| Name | Signature | Effect |
| --- | --- | --- |
| `h` | `h(qubit)` | Hadamard. |
| `x` | `x(qubit)` | Pauli-X. |
| `y` | `y(qubit)` | Pauli-Y. |
| `z` | `z(qubit)` | Pauli-Z. |
| `rx` | `rx(qubit, float radians)` | Rotation about X by `radians`. |
| `ry` | `ry(qubit, float radians)` | Rotation about Y by `radians`. |
| `rz` | `rz(qubit, float radians)` | Rotation about Z by `radians`. |
| `cx` | `cx(qubit control, qubit target)` | Controlled-X. |

### Measurement and reset
- `measure q` returns `bit` and collapses `q`.
- `reset q` returns `q` to \(|0\rangle\) and clears its measured flag.

### Diagnostics
- `echo(value)` prints classical data; suppressed on intermediate shots unless `--echo=all` is set.

### Notes
- Gate names are reserved.
- Additional gates land through compiler releases; 1.1.x ships the set above.
