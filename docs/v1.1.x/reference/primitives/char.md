---
title: char
---

Single-byte character.

### Literals
- `'a'`, `'Z'`, `'0'`. Escape sequences are not parsed in 1.1.x.

### Usage
```bloch
char delim = ',';
string label = "q0" + delim + "q1";
```

### Notes
- Primarily used inside strings; concatenation is performed on `string` values.
