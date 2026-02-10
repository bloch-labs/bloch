# ADR-007: Stdlib Collections Package — Target v1.2.0

## Owner
Akshay Pal

## Status
Proposed

## Delivered In
Planned for v1.2.0

## Context
- With boxing (ADR-006), primitives can be represented as objects. We need a coherent collections surface that operates on reference types (boxed primitives and user classes).
- Goal: provide familiar, minimal, deterministic collections akin to Java’s core types while keeping runtime/simple semantics.

## Decision
- Introduce the following collections packages:
  - `List<T>`
  - `Set<T>`
  - `Map<K, V>`
  - `Pair<A, B>`
- Type arguments are reference-oriented (boxed primitives or user classes); with implicit `Object` inheritance, explicit `T extends Object` bounds are unnecessary in API signatures. Primitives themselves are not accepted.
- No concurrent collections, no iterators/generators in v1.2.0; APIs are strict and eager.
- Align with the explicit boxing model and avoid primitive specialisation complexity.
- Keep API surface small yet cover common needs (ordered list, uniqueness set, key/value map, simple pair tuple).

## Alternatives Considered
- Allow primitives directly — **rejected** because it would require specialisation and inflate surface area.
- Add iterators/generators now — **deferred** to keep v1.2.0 scope tight and implementations simple.

## Consequences
- Requires stdlib implementations and tests for the four types.
- Examples and docs must show usage with boxed primitives (e.g., `List<Integer>`) and user classes.
- Runtime unchanged; **all logic is library-level**.

## References
- ADR-001 Class System
- ADR-003 Class Generics
- ADR-006 Boxed Primitives
