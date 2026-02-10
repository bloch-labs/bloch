---
title: Annotations
---
# Annotations

Bloch uses annotations to mark functions or variables with special behaviour. Annotations are prefixed with `@` and appear immediately before the item they modify.

Available annotations in v1.0.0:

- `@quantum` — marks a function that performs quantum operations, functions with the `@quantum` annotation can only return `void` or `bit`.
- `@tracked` — collects outcome statistics for qubits across program runs.

Multiple annotations are allowed syntactically, but each has specific applicability and rules described below.

## `@quantum` functions

Apply `@quantum` to functions that call quantum gates (`h, x, y, z, rx, ry, rz, cx`) or otherwise manipulate qubits.

Rules:
- Return type must be `bit` or `void`.
- Non-`void` `@quantum` functions must return along all paths (usual rule applies).

Example:

```
@quantum
function bell(qubit a, qubit b) -> void {
  h(a); cx(a, b);
}

@quantum
function sample() -> bit {
  qubit q; h(q);
  return measure q;
}
```

## `@tracked` qubits

Use `@tracked` on `qubit` or `qubit[]` variables to aggregate measurement outcomes across runs (shots). Tracking ends at scope exit.

Rules:
- Only valid on `qubit` or `qubit[]`. Applying it to other types is a semantic error.
- If a tracked qubit/register is never measured, an outcome of `"?"` is recorded.

Examples:

```
function main() -> void {
  @tracked qubit q;
  h(q);
  measure q; // record 0/1 for this shot
}

function two() -> void {
  @tracked qubit[2] r;
  h(r[0]); cx(r[0], r[1]);
  measure r[0]; measure r[1];
}
```

Run with multiple shots to see an aggregate table in the CLI output:

```
bloch --shots=100 entangle.bloch
```

Tip: use `--echo=all` to print `echo()` output on every shot (otherwise suppressed when `--shots > 1`).
