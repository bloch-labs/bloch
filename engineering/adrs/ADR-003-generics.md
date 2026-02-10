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
- Generics are class-level only.
- Syntax:
  - Declaration: `class Foo<T>`
  - Upper bound: `class Foo<T extends Bar>`
  - Use-site: `Foo<Baz>`, including in `extends`.
- Bounds are upper bounds against class types in the single-inheritance hierarchy.
- Classes without explicit `extends` still participate in the implicit `Object`-rooted hierarchy.
- Runtime uses monomorphisation with specialisation caching.
- Constructor-call diamond inference is supported when the target type is known
  (typed declarations, assignments, and returns).
- Nullability unchanged: class references (including generic instantiations) are nullable;
  primitives/arrays are not.

## Explicitly Not Included
- Method-level generics.
- Wildcard generics: `?`, `? extends`, `? super`.
- Declaration-site variance markers.
- Raw types.
- Method-argument-driven generic inference.
- Generic arrays.

## Alternatives Considered
- Method-level generics — **rejected** due to added parser/runtime complexity for limited gain.
- Wildcards / variance annotations — **rejected** to keep surface small and avoid variance rules in the first iteration.
- Type erasure instead of monomorphisation — **rejected** to preserve runtime type fidelity and dispatch speed.

## Implementation Notes (v1.1.0)
- Parser and semantic analysis reject wildcard syntax by omission from the grammar.
- Semantic/runtime call and assignment rules use subtype assignability within the
  single-inheritance hierarchy.

## Consequences
- Code size can grow with many specialisations; caching mitigates.
- No generic functions; patterns needing them must wrap in generic classes or overloads.
- Base classes may be generic; specialisations propagate through `extends`.
