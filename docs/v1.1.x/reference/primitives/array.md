---
title: arrays
---

Fixed-size, typed arrays. Elements may be primitives or class references.

### Declaration
```bloch
int[4] counts;
qubit[2] pair;
final int n = 3;
float[n] angles;
MyClass[2] objects;
```
Lengths must be compile-time constants.

### Literals and indexing
```bloch
bit[2] basis = {0b, 1b};
float[3] vec = {0.0f, 0.5f, 1.0f};
MyClass[2] items = {new MyClass(), new MyClass()};
```
Zero-based indexing; out-of-bounds access fails at runtime.

### Notes
- Arrays of qubits can be annotated `@tracked`: `@tracked qubit[4] q;`.
- Arrays cannot be resized; there is no slicing syntax.
