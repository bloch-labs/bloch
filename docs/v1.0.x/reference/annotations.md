---
title: Annotations
---

Annotations modify function or variable behavior.

### @quantum (functions)
Marks a function as quantum-only.
```bloch
@quantum
function oracle(qubit a, qubit b) -> void {
    cx(a, b);
}
```
Constraints:
- Return type must be `void`, `bit`, or `bit[]`.
- `main` cannot be annotated `@quantum`.

### @tracked (variables)
Accumulates measurement histograms when running multiple shots.
```bloch
function main() -> void {
    @tracked qubit[2] q;
    h(q[0]); cx(q[0], q[1]);
    bit[2] out = {measure q[0], measure q[1]};
    echo(out);
}
```
Use with `--shots=N` to emit aggregated outcome tables. Available only on `qubit` and `qubit[]`.
