---
title: Functions
---

Declarative units of reusable logic. All Bloch code executes inside functions.

### Syntax
```bloch
function name(paramType param, ...) -> returnType {
    // body
}
```

### Rules
- Parameters and return types are mandatory; no overloading.
- `return` is required on non-`void` paths.
- `@quantum` functions may only return `void`, `bit`, or `bit[]` and cannot be `main`.
- Functions cannot be nested or declared inside other blocks.

### Example
```bloch
@quantum
function rotate(qubit q, float theta) -> void {
    ry(q, theta);
}

function main() -> void {
    qubit q;
    rotate(q, 0.5f);
    bit b = measure q;
    echo(b);
}
```

### Contracts
- Arguments are passed by value for classical types and by handle for qubits; mutations to qubits affect the caller.
- There is no recursion limit enforced by the language, but simulator state growth may dominate.
