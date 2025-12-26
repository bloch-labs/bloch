---
title: float
---

Floating point type (matches host `double` for 1.0.x).

## Literals
- Must start with a digit and end with `f`.
- Examples: `1.0f`, `-0.25f`, `3f`.
- `.5f` is invalid (no leading-dot form); a decimal without `f` is invalid (e.g., `1.0` fails to lex).
- Exponent notation and hex floats are not supported.

## Usage
- Declarations: `float theta = 0.5f;`
- Arrays: `float[4] amps;`, `float[] dyn;`
- Operators: arithmetic, comparison, logical (`&& || !`), and bitwise are **not** allowed for `float`.
- Conversions: `int` and `bit` values are promoted to `float` in arithmetic.

## Errors
- Division by zero is a runtime error.
- Out-of-bounds indexing on `float[]` is a runtime error.
