---
title: Reference
---

Reference index for Bloch v1.0.x (as of the initial 1.0.x release).

## Primitives and literals
- [`int`](int.md) — signed integer. Literals are decimal digits (e.g., `0`, `42`, `-5`).
- [`float`](float.md) — floating point. Literals **must end with `f`** (e.g., `1.0f`, `3f`); a decimal without `f` is invalid.
- [`bit`](bit.md) — classical bit. Literals: `0b` or `1b`.
- [`char`](char.md) — single character. Literals: `'a'`, `'Z'` (no escape sequences).
- [`string`](string.md) — double-quoted string. Literals: `"hello"` (no escape sequences; may span lines).
- [`qubit`](qubit.md) — quantum handle; no literals.
- [`arrays`](arrays.md) — fixed/dynamic arrays of the above primitives (including `qubit`).

## Keywords
- See [`keywords`](keywords.md).

## Annotations
- See [`annotations`](annotations.md).

## Classes
- See [`Classes`](../classes.md) for the status in 1.0.x (not supported; guidance included).

## Quantum operations
| Name | Signature | Description |
| ---- | ---------- | ----------- |
| `h`  | `h(qubit)` | Hadamard |
| `x`  | `x(qubit)` | Pauli-X |
| `y`  | `y(qubit)` | Pauli-Y |
| `z`  | `z(qubit)` | Pauli-Z |
| `rx` | `rx(qubit, float theta)` | Rotation around X by `theta` radians |
| `ry` | `ry(qubit, float theta)` | Rotation around Y by `theta` radians |
| `rz` | `rz(qubit, float theta)` | Rotation around Z by `theta` radians |
| `cx` | `cx(qubit control, qubit target)` | Controlled-NOT |

## Lifetime and measurement rules
- Measuring a qubit marks it as measured; re-measuring without `reset` is an error.
- `reset q;` returns the qubit to `|0⟩` and clears the measured marker.
- Unmeasured qubits at scope exit emit warnings (suppressed on intermediate shots when `--shots` > 1).
- `@tracked` variables aggregate measurement outcomes at scope exit; results are reported after multi-shot runs.

## CLI
- `bloch [options] <file.bloch>`
- Options:
  - `--shots=N` — repeat execution N times; aggregates tracked counts/probabilities.
  - `--emit-qasm` — print generated OpenQASM (also writes `<file>.qasm` next to the source).
  - `--echo=all|none` — override echo suppression (default: suppress when shots > 1).
  - `--update` — self-update to the latest release when available.
  - `--version` — print version and run a background update check.
  - `--help` — usage summary.

## Outputs and errors
- Outputs: `<file>.qasm` alongside the source; tracked summaries when `--shots` > 1; echo of classical values (buffered).
- Errors: parse (syntax), semantic (type/lifetime), runtime (division by zero, out-of-bounds, invalid measurement target).
- Warnings: unmeasured qubits on scope exit; missing tracked data when nothing is tracked.
