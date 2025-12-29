---
title: int
---

Signed integer (host-sized) for classical control, counters, and array dimensions.

### Literals
- Decimal digits only: `0`, `42`, `1024`.

### Usage
```bloch
int cycles = 10;
for (int i = 0; i < cycles; i++) { echo(i); }
final int width = 4;
int[width] data = {0, 1, 2, 3};
```

### Operators
`+`, `-`, `*`, `/`, `%`, comparisons, prefix/postfix `++`/`--`, ternary conditionals. Division truncates toward zero.

### Notes
- Array lengths must be integer expressions known at compile time (literals or `final` integers).
- Mixed `int`/`float` expressions promote to `float`.
