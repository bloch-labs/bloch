---
title: arrays
---

Fixed-size, typed arrays of any primitive (`int`, `float`, `bit`, `char`, `string`, `qubit`).

### Declaration
```bloch
int[4] counts;
qubit[2] pair;
final int n = 3;
float[n] angles;
```
Array lengths must be compile-time constants.

### Literals and indexing
```bloch
bit[2] basis = {0b, 1b};
float[3] vec = {0.0f, 0.5f, 1.0f};
qubit[3] q;
rx(q[0], 0.25f);
bit b = measure q[1];
```
Indices are zero-based. Out-of-bounds access fails at runtime.

### Notes
- Arrays of qubits can be annotated `@tracked` as a whole: `@tracked qubit[4] q;`.
- Arrays are value containers; there is no slicing or dynamic resize in 1.0.x.
