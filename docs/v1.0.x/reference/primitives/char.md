---
title: char
---

Single-byte character.

### Literals
- `'a'`, `'Z'`, `'0'`. Escape sequences are not supported in 1.0.x.

### Usage
```bloch
char delim = ',';
string line = "a,b,c";
```

### Notes
- Characters are mainly useful inside strings; string concatenation uses `string` operands.
