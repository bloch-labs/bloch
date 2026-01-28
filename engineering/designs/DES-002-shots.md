# DES-002: @shots Annotation Design

## Scope
Implements ADR-004: source-level shot count annotation on `main`.

## High Level Flow
- Parser reads `@shots(N)` on `main` → Semantics validates placement/value → Runtime resolves shot count vs CLI → Execution uses resolved count.

## Compilation (Lexer/Parser/AST)
- `@shots(N)` parsed as function annotation; allowed on `main` only.
- `N` must be an integer literal; stored on annotation node.

## Semantic Analysis
- Reject `@shots` on non-`main` or multiple occurrences.
- Validate `N` is integer literal.
- Ensure `main` return type is compatible with `@quantum` rules when used together (`bit`, `bit[]`, `void`).

## Runtime Architecture
- Shot resolution order: annotation > CLI `--shots` > default.
- When both CLI and annotation differ, emit warning and use annotation value.
- Execution loops over resolved shot count, aggregates tracked values, and respects echo suppression defaults.

## Testing
- Semantic tests: placement, multiplicity, literal checks, invalid types.
- Integration: annotation alone; CLI alone; conflicts produce warning and use annotation.

## Compatability
- Backward-compatible: CLI still works; annotation is preferred when present. No impact on non-quantum programs beyond shot handling.
