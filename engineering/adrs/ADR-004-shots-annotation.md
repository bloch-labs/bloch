# ADR-004: @shots Annotation

## Owner
Akshay Pal

## Status
Accepted

## Delivered In
v1.1.0

## Context
Bloch supports repeated execution of quantum programs (“shots”). We needed a source-level way to specify the number of shots per program, with precedence rules relative to CLI flags and safe integration with `@quantum` functions.

## Decision
- Introduce `@shots(N)` annotation on the `main` function only.
- If both CLI `--shots` and `@shots(N)` are present, `@shots` has higher precedence and a warning is emitted about the mismatch.
- `@shots` is rejected on non-`main` functions; `main` may also be `@quantum` if its return type is valid (`bit/bit[]`/`void`).
- Semantic checks enforce placement and validate that `N` is an integer literal.
- Runtime uses the resolved shot count when executing the entry point.

## Alternatives Considered
- Only CLI flag — **rejected** due to poor portability/reproducibility; keeps config outside source.

## Consequences
- Users get deterministic shot selection; conflicts produce explicit warnings.
- No per-function or per-block shot overrides—out of scope for simplicity.
- CLI flag to be deprecated in v2.0.0. 
