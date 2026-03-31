# Built-ins

Bloch ships a small set of built-ins. These are treated like functions and validated by the
semantic analyser.

## Console
- `echo(expr) -> void` — prints a human-friendly representation of `expr` to stdout.

## Quantum gates
All return `void`.
- `h(qubit)`
- `x(qubit)`
- `y(qubit)`
- `z(qubit)`
- `rx(qubit, float)`
- `ry(qubit, float)`
- `rz(qubit, float)`
- `cx(qubit, qubit)`

## Measurement/Reset
- `measure q` — expression form, accepts a single `qubit` and returns `bit`.
- `measure q;` — statement form, accepts `qubit` or `qubit[]`.
- `reset q;` — statement; accepts a single `qubit` and sets it to `|0>`.

See also: [Language Specification](../../docs/language-spec.md).
