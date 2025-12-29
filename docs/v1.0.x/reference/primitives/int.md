---
title: int
---

Signed integer value (host-sized). Intended for counters, array indices, and classical control.

### Literals
- Decimal digits only (e.g., `0`, `42`, `1024`).
- No binary/hex/octal prefixes in 1.0.x.

### Usage
```bloch
int cycles = 10;
for (int i = 0; i < cycles; i++) { echo(i); }
final int width = 3;  // compile-time constant for array sizes
int[width] data = {0, 1, 2};
```

### Operators
`+`, `-`, `*`, `/`, `%`, comparison operators, prefix/postfix `++`/`--`, and ternary conditionals. Division is integer division.

### Notes
- Array bounds must be integer expressions known at compile time (literals or `final` integers).
- Implicit widening to `float` occurs via normal arithmetic; there is no implicit conversion from `float` to `int`.
