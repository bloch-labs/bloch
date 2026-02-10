# ADR-006: Stdlib - Boxed Classes for all Primitives

## Owner
Akshay Pal

## Status
Proposed

## Delivered In
Planned for v1.2.0

## Context
- This ADR establishes a boxed class for **every primitive** supported by the language.
- We want primitives for performance/representation and boxed references for richer APIs, without adding auto-boxing complexity.
- Quantum primitives (`bit`, `qubit`) are *not* boxed in v1.2.0.

## Decision
- Create a boxed class per primitive:
  - `Boolean` (`boolean`)
  - `Integer` (`int`)
  - `Long` (`long`)
  - `Float` (`float`)
  - `Char` (`char`)
  - `String` (`string`)
  - The `qubit` primitive is deliberately excluded from boxing; quantum state is not safely encapsulated as an object. `bit` remains unboxed; use `Boolean` for classical logic.
- Each boxed class extends `Object` and provides at least:
  - Static `of(<prim>)`
  - `override function toPrimitive() -> <prim>` (Object declares `virtual function toPrimitive() -> void` placeholder)
  - `equals` override for **value equality** with same-kind boxes
  - `toString` override for rendering the contained value
- No automatic boxing/unboxing in v1.2.0; conversions are explicit via constructors/accessors.

## Alternatives Considered
- Auto-boxing/unboxing  — **rejected** to preserve explicit semantics, avoid hidden allocations, and keep runtime simpler for v1.2.0.

## Consequences
- Stdlib surface expands with boxed classes; documentation and examples updated.
- Runtime unchanged; boxes are ordinary classes.
- Slight verbosity for users (must call `of` / `toPrimitive`) but predictable behaviour.

## References
- ADR-001 Class System
- ADR-002 Null Handling
- ADR-003 Class Generics
- ADR-005 New Primitives
