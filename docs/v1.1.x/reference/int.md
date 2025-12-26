---
title: int
---

Signed integer type (implementation-defined width; matches the host `int` for 1.1.x).

## Literals
- Decimal digits: `0`, `42`, `-5`.

## Usage
- Declarations: `int count = 0;`
- Arrays: `int[4] data;`, `int[] dyn;`
- Operators: arithmetic, comparison, logical, and bitwise operations apply; `%` is defined.
- Conversions: `bit` promotes to `int` when used in arithmetic (`bit` 0/1 becomes 0/1). `float` is not implicitly converted; explicit assignment from float truncates at runtime.

## Errors
- Division/modulo by zero is a runtime error.
- Out-of-bounds indexing on `int[]` is a runtime error.
