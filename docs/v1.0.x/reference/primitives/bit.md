---
title: bit
---

Classical single-bit value. Produced by measurements and used in classical control.

### Literals
- `0b` or `1b` only.

### Usage
```bloch
bit outcome = measure q;
if (outcome) { z(q); }
bit[2] pair = {measure a, measure b};
```

### Notes
- Bits are integers under the hood (`0` or `1`) and support comparisons and equality operators.
- Use arrays of bits to capture multi-qubit measurement results.
