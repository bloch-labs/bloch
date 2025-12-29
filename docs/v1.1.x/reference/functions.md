---
title: Functions
---

Declarative units of reusable logic. Applies to free functions; see [Classes](classes.md) for methods.

### Syntax
```bloch
function name(paramType param, ...) -> returnType {
    // body
}
```

### Rules
- Explicit parameter and return types; no overloading.
- Non-`void` functions must return on all paths.
- `@quantum` functions may only return `void`, `bit`, or `bit[]` and cannot be `main`.
- Only one `main` may exist across all imported modules.
- Functions cannot be nested.

### Example
```bloch
@quantum
function oracle(qubit a, qubit b) -> void { cx(a, b); }

function main() -> void {
    qubit[2] q;
    oracle(q[0], q[1]);
    bit[2] out = {measure q[0], measure q[1]};
    echo(out);
}
```

### Contracts
- Arguments are passed by value for classical types and by handle for qubits.
- `@shots` is valid only on `main` and sets the authoritative shot count.
