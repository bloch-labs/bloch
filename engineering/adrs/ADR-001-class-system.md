# ADR-001: Bloch Class System

## Owner
Akshay Pal

## Status
Accepted

## Delivered In
v1.1.0

## Context
Bloch is moving toward Java-style execution in v2.0.0, so the OO model needs a
clear and strict contract now. This ADR defines inheritance, dispatch,
construction, visibility, and the relationship to nullability and generics.
Detailed sub-decisions: ADR-002 (null), ADR-003 (generics), ADR-009 (imports).

## Decision
- Single inheritance only: one optional `extends` clause.
- Every non-static class has implicit root base `Object`.
  - `class Animal {}` is semantically treated as `class Animal extends Object {}`.
- Root `Object` is provided by stdlib module `bloch.lang.Object`, auto-loaded by
  the module loader when available (with a synthetic fallback root for minimal runtime safety).
- Java-style subclass families must remain valid:
  - `class Animal {}`
  - `class Dog extends Animal {}`
  - `class Cat extends Animal {}`
- Modifiers: `abstract` and `static` at class level.
- Members: `public`, `protected`, `private`; methods may be `virtual`/`override`.
- Construction/destruction:
  - `super(...)` only as first constructor statement.
  - At most one destructor per class.
  - Static classes cannot have instance fields/constructors/destructors.
- Nullability and generics behaviour are governed by ADR-002 and ADR-003.

## Included (Contract)
- Single inheritance hierarchy rooted at `Object`.
- Virtual dispatch through an override chain.
- Static and instance members with Java-like visibility rules.
- Class-level generics and bounded type parameters.

## Explicitly Not Included (Contract)
- Multiple inheritance.
- Interfaces and traits.
- Method-level generics.
- Wildcard generics (`?`, `? extends`, `? super`).
- Raw types and constructor type inference.
- Reflection and runtime annotation processing.

## Alternatives Considered
- Multiple inheritance or mixins — **rejected** due to increased layout/vtable complexity and ambiguity resolution cost.
- Allow static virtual/override — **rejected** due to dispatch model mismatch; static members have no runtime receiver/vtable.

## Consequences
- Predictable, Java-oriented OO surface for v2.0.0 execution goals.
- A strict "in-scope/out-of-scope" boundary for language users and implementers.
- Consistent semantic/runtime behaviour for implicit `Object`, constructor defaults,
  and subtype assignability.
