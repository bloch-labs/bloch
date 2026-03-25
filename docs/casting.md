# Casting

Bloch supports explicit casts using parentheses and performs implicit float promotion for
division (for example, `1/2` yields `0.5f`).

## Syntax
- Cast: `(target_type) expression`
- Supported targets: `int`, `long`, `float`, `bit`
- Not supported: `void`, `boolean`, `char`, `string`, or class types

## Numeric rules
- Division `/` always produces a `float` result when used with numeric primitives, even if both
  operands are `int` or `bit`.
- `%` remains integer-only.
- Explicit casts allow:
  - Widening: `int`/`long`/`bit` -> `float`
  - Narrowing: `float` -> `int`/`long`/`bit` (truncates toward zero)
  - Integer widening: `int` -> `long`

## Examples
```bloch
function main() -> void {
    echo(1/2);               // 0.5 (implicit promotion)

    int numerator = 3;
    int denominator = 2;
    echo((float)numerator / denominator); // 1.5

    float widened = (float)numerator;
    int truncated = (int)widened;         // 3
    bit flag = (bit)widened;              // 1 (non-zero becomes 1)
    long big = (long)numerator;           // 3L
}
```
