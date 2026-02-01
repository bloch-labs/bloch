# ADR-005: New Primitives (boolean, long)

## Owner
Akshay Pal

## Status
Accepted

## Delivered In
v1.1.0

## Context
- Current primitives: `int`, `float`, `bit`, `char`, `string`, `qubit` (+ arrays).
- There is demand for classical booleans (distinct from measurement `bit`) and wider integer math (signed 64-bit).
- To keep the language lean, we add a primitive only when it adds unique representation/semantics not covered by classes or existing primitives.

## Decision
- Introduce two new primitives:
  1. `boolean` — classical logic type; literals `true/false`; used for conditionals, logical ops; distinct from `bit` to avoid quantum/classical conflation.
  2. `long` — 64‑bit signed integer. Literal suffix `L` (e.g., `42L`).

## Alternatives Considered
- Use `bit` as boolean — **rejected** due to conflating quantum measurement with classical logic and creating implicit conversions.
- Rely solely on boxed types — **rejected** due to performance/ergonomics; primitives provide faster arithmetic and leaner storage.

## Consequences
- Parser/lexer add keywords and literal handling (`true/false`, `L` suffix).
- Semantics add `ValueType::Boolean/Long`; type-checking rules for promotion and operator validity.
- Runtime adds new value kinds and array variants; QASM emission unchanged for classical flow.
- Backward compatibility: existing code unaffected; `bit` semantics unchanged.
