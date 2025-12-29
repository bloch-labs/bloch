---
title: bit
---

Classical single-bit value used for measurement results and branching.

### Literals
- `0b` or `1b` only.

### Usage
```bloch
bit outcome = measure q;
if (outcome) { z(q); }
bit[2] pair = {measure a, measure b};
```

### Notes
- Bits are integers (`0`/`1`) for comparison and concatenation into strings.
- Aggregate multi-qubit outcomes with `bit[]` arrays.
