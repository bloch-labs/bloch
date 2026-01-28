# ADR-003: Class Generics

## Owner
Akshay Pal

## Status
Accepted

## Delivered In
v1.1.0

## Context
Bloch requires reusable, type-safe class components (e.g., containers) without runtime type erasure. Requirements include class-level type params, optional bounds, predictable dispatch, compatibility with existing runtime, minimal surface (no method generics/wildcards).

## Decision
- Generics are class-level only; no method generics, no wildcards, no constructor inference.
- Syntax: `class Foo<T>`; bounds via `class Foo<T extends Bar>`.
- Type arguments allowed on use and in `extends`: `Foo<int> f; class Baz<T> extends Foo<T> {}`.
- Runtime monomorphisation: first use of each type-argument set creates a concrete specialisation; cached thereafter.
- Nullability unchanged: class refs (including generics) nullable; primitives/arrays not.

## Alternatives Considered
- Method-level generics — **rejected** due to added parser/runtime complexity for limited gain.
- Wildcards / variance annotations — **rejected** to keep surface small and avoid variance rules in the first iteration.
- Type erasure instead of monomorphisation — **rejected** to preserve runtime type fidelity and dispatch speed.

## Consequences
- Code size can grow with many specialisations; caching mitigates.
- No generic functions; patterns needing them must wrap in generic classes or overloads.
- Base classes may be generic; specialisations propagate through `extends`.
