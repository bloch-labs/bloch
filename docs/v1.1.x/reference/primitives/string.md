---
title: string
---

Immutable sequence of characters for classical text and diagnostics.

### Literals
- Double-quoted: `"hello"`, `"q0"`.
- Escape sequences are not supported; include raw characters.

### Usage
```bloch
string name = "Bloch";
echo("Hello, " + name);
bit b = measure q;
echo("Measured: " + b);
```

### Notes
- Concatenation uses `+` and coerces `int`, `float`, and `bit`.
- Strings participate only in classical logic; they cannot be tracked or annotated.
