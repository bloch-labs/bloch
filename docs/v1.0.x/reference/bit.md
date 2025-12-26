---
title: bit
---

Classical bit (0 or 1).

## Literals
- `0b`
- `1b`
- Any other digits with `b` are invalid.

## Usage
- Declarations: `bit flag = 1b;`
- Arrays: `bit[2] pair;`, `bit[] dyn;`
- Operators: logical (`&& || !`), bitwise (`& | ^ ~`), comparisons, arithmetic (+/-/*//) with implicit promotion to `int` when used numerically.
- Assignment: type-checked at runtime; non-bit values in `bit[]` assignment raise errors.

## Errors
- Out-of-bounds indexing on `bit[]` is a runtime error.
- Bitwise ops between `bit[]` require equal lengths (enforced at runtime).
