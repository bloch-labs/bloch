# ADR-001: Bloch Class System

## Owner
Akshay Pal

## Status
Accepted

## Delivered In
v1.1.0

## Context
Defines the overall class model for Bloch, covering inheritance, dispatch, construction, visibility, and interaction with nullability, generics, and entry semantics. Detailed sub-decisions can be found in ADR-002 (Null) and ADR-003 (Generics).

## Decision
- Single inheritance (`extends Base`, optional type args on the base).
- Modifiers: `abstract` (non-instantiable), `static` (type-only container).
- Members: visibility `public` (accessible everywhere), `protected` (class + subclasses), `private` (declaring class only); methods may be `virtual`/`override`; overloading by arity/types allowed; static cannot be virtual/override.
- Construction/destruction: non-static classes must declare a ctor; `super(...)` only first statement; one destructor max; static classes cannot have ctors/dtors/instance fields.
- Nullability and generics behaviours are governed by ADR-002/003.

## Alternatives Considered
- Multiple inheritance or mixins — **rejected** due to increased layout/vtable complexity and ambiguity resolution cost.
- Implicit/default ctors for all classes — **rejected** due to hidden initialisation semantics; explicit ctors keep field binding clear.
- Allow static virtual/override — **rejected** due to dispatch model mismatch; static members have no runtime receiver/vtable.

## Consequences
- Predictable, Java-like OO model; easy to reason about layout and dispatch.
- Scope-limited features reduce runtime complexity while permitting generics and null checks via referenced ADRs.
