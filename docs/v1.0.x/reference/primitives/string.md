---
title: string
---

Immutable sequence of characters for classical text and diagnostics.

### Literals
- Double-quoted: `"hello"`, `"q0"`.
- Escape sequences are not parsed in 1.0.x; include characters directly.

### Usage
```bloch
string name = "Bloch";
echo("Hello, " + name);
bit b = measure q;
echo("Measured: " + b);
```

### Notes
- Concatenation uses `+` and coerces `int`, `float`, and `bit` to text.
- Strings cannot be annotated `@tracked` and have no quantum meaning.
