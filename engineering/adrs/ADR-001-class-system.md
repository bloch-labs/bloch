# ADR-001: Bloch Class System

## Owner
Akshay Pal

## Status
Accepted

## Delivered In
v1.1.0

## Context
This ADR defines the core class system of Bloch including inheritance, dispatch,
construction, visibility, and the relationship to nullability and generics.
Detailed sub-decisions: ADR-002 (null), ADR-003 (generics), ADR-009 (imports).

## Decision
- Single inheritance only: one optional `extends` clause.
- Every non-static class has implicit root base `Object`.
  - `class Animal` is semantically treated as `class Animal extends Object`.
- Root `Object` is provided by stdlib module `bloch.lang.Object`, auto-loaded by
  the module loader when available (with a synthetic fallback root for minimal runtime safety).
- Subclass families must remain valid:
  - `class Animal { public constructor() -> Animal = default; }`
  - `class Dog extends Animal { public constructor() -> Dog { super(); return this; } }`
  - `class Cat extends Animal { public constructor() -> Cat { super(); return this; } }`
- Modifiers: `abstract` and `static` at class level.
- Members: `public`, `protected`, `private`; methods may be `virtual`/`override`.
- Construction/destruction:
  - Every non-static class must declare at least one constructor.
  - `super(...)` only as first constructor statement.
  - If `super(...)` is omitted, an accessible zero-arg base constructor is required.
  - At most one destructor per class.
  - Static classes cannot have instance fields/constructors/destructors.
- Nullability and generics behaviour are governed by ADR-002 and ADR-003.

## Included
- Single inheritance hierarchy rooted at `Object`.
- Virtual dispatch through an override chain.
- Static and instance members with visibility rules.
- Class-level generics and bounded type parameters.
- Generic diamond inference for constructor calls when assignment target type is known.

## Explicitly Not Included
- Multiple inheritance.
- Interfaces and traits.
- Method-level generics.
- Wildcard generics (`?`, `? extends`, `? super`).
- Raw types and method-argument-driven generic inference.
- Reflection and runtime annotation processing.

## Alternatives Considered
- Multiple inheritance or mixins — **rejected** due to increased layout/vtable complexity and ambiguity resolution cost.
- Allow static virtual/override — **rejected** due to dispatch model mismatch; static members have no runtime receiver/vtable.

## Consequences
- Predictable  OO surface for v2.0.0 execution goals.
- A strict "in-scope/out-of-scope" boundary for language users and implementers.
- Consistent semantic/runtime behaviour for implicit `Object`, explicit constructor
  requirements, and subtype assignability.
