---
title: arrays
---

Arrays of primitives (including `qubit`).

## Forms
- Fixed size: `int[4] counts;`, `qubit[2] regs;`
- Dynamic size: `float[] amps;`, `bit[] flags;` (size may be set at runtime and is validated when used)

## Literals
- Classical arrays: `{1, 2, 3}`, `{"a", "b"}`, `{1b, 0b}`
- Type-checked at runtime; element types must match the declared element type.
- `qubit[]` cannot be initialised via literals.

## Indexing
- `arr[i]` is bounds-checked at runtime.
- Negative indices are rejected at parse time.
- Assignment into arrays is type-checked at runtime (mismatches raise errors).

## Operations
- `bit[]` supports elementwise `& | ^ ~` with size checks.
- Other array element operations follow their element types (e.g., arithmetic on `int[]` elements).

## Errors
- Out-of-bounds access is a runtime error.
- Type-mismatched literal or assignment elements raise runtime errors.
