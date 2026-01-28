---
title: Built-ins
---
# Built-ins

Bloch ships a minimal set of built-ins in v1.0.0 RC.

## Console

- `echo(expr) -> void` — prints a human-friendly representation of `expr` to stdout. Strings are concatenated with `+`.

Quick tip: use `--echo=all` when running multi-shot to see per-shot output.

## Quantum gates

These are treated like functions and validated by the semantic analyser. All return `void`.

- `h(qubit)`
- `x(qubit)`
- `y(qubit)`
- `z(qubit)`
- `rx(qubit, float)`
- `ry(qubit, float)`
- `rz(qubit, float)`
- `cx(qubit, qubit)`

See also: [Semantic Rules](../language/semantics) for signature checking.

## Measurement/Reset

- `measure q` — expression form, returns `bit`.
- `measure q;` — statement form.
- `reset q;` — statement; sets `q` to `|0>`.

