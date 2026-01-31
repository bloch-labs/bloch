# Casting

Bloch v1.1.x supports explicit casting using parentheses and performs implicit float promotion for division e.g. `1/2` yields `0.5`.

## Syntax
- Cast: `(target_type) expression`
- Supported targets: `int`, `float`, `bit`
- Not supported: `void`, `char`, or class types

## Numeric rules
- Division `/` always produces a `float` result when used with numeric primitives, even if both operands are `int`.
- `%` remains integer-only.
- Explicit casts allow:
  - Widening: `int`/`bit` → `float`
  - Narrowing: `float` → `int`/`bit` (truncates toward zero)

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
}
```