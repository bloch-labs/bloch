# ADR-002: Null Handling

## Owner
Akshay Pal

## Status
Accepted

## Delivered In
v1.1.0

## Context
Disciplined representation of “no object” while keeping primitives and arrays non-nullable. To be used for explicit comparisons, safe member access, and consistent compile-time/runtime rules.

## Decision
- Introduce `null` literal.
- Allow `null` only for class references (including generics); disallow for primitives and arrays.
- Permit `==` / `!=` comparisons involving `null`; reject other operators with null.
- Member access on `null` raises `"null reference"` at runtime.
- `destroy null` is accepted as a no-op for defensive cleanup.

## Alternatives Considered
- Allow null on primitives/arrays — **rejected** due to safety concerns and added checks on every primitive use.
- Implicit coercions involving null — **rejected** due to surprising conversions and loss of explicitness.

## Consequences
- Arrays and primitives remain non-nullable; class refs are nullable.
- Users must guard member access when a reference may be `null`.
- Consistent semantics across parser/semantic/runtime layers.
