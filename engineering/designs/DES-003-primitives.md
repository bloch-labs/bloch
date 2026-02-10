# DES-003: New Primitives (boolean, long)

## Scope
* ADR-005

## High Level Flow
- Lexer adds keywords/literal suffixes → Parser recognises `boolean`/`long` primitives and `L` suffix → Semantics enforces type rules/promotions → Runtime adds value kinds/ops/printing.

## Compilation (Lexer/Parser/AST)
- Lexer: keywords `boolean`, `true`, `false`; literal suffix `L` for long.
- Parser: `PrimitiveType` accepts `boolean` and `long`; literals tagged with widened kind.
- AST touchpoints: `token.hpp`, `lexer.cpp`, `parser.cpp`.

## Semantic Analysis
- `ValueType` adds `Boolean`, `Long`.
- Promotions: `int + long -> long`; `long` with `float -> float`; `boolean` only in logical ops/conditions.
- Control flow: `boolean` (and `bit` for backward compatibility) allowed in conditionals; consider future lint against `bit` in classical predicates.
- Nullability: unchanged; primitives remain non-nullable.

## Runtime Architecture
- `Value::Type` adds `Boolean`, `Long` plus array variants (`BooleanArray`, `LongArray` where needed).
- Storage: `Boolean` compact; `Long` 64-bit.
- Ops: arithmetic/bitwise for `long` mirror `int` but 64-bit; logical ops on `boolean`.
- Echo/QASM: `boolean` prints `true/false`.

## Testing
- Parser: literals (`true/false`, `123L`), type parsing, mixed expressions.
- Semantics: invalid mixes (`boolean + int`), promotions, control-flow acceptance.
- Runtime: `long` edge cases (overflow behaviour), boolean logic.
- Integration: example programs using new primitives and boxed counterparts.

## Compatability
- Existing code unaffected; `bit` semantics unchanged. No auto-widening except via explicit long operations; literals without `L` stay `int`.
