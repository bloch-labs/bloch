# Contributor Guide: Bloch Class System

Audience: contributors extending/modifying the class system. Read ADR-001 (umbrella), ADR-002 (null), ADR-003 (generics), ADR-004 (@shots) and designs DES-001, and DES-002 before making changes.

## Where Things Live
- AST/Parser: `src/bloch/core/ast/ast.hpp`, `src/bloch/core/parser/parser.{hpp,cpp}`
- Semantics: `src/bloch/core/semantics/semantic_analyser.{hpp,cpp}`
- Runtime: `src/bloch/runtime/runtime_evaluator.{hpp,cpp}`
- Tests: `tests/test_ast.cpp`, `tests/test_parser.cpp`, `tests/test_semantics.cpp`, `tests/test_runtime.cpp`, `tests/test_integration.cpp`

## Extension Checklist
1) **AST/Parser**: Add syntax; update `isTypeAhead`, cloning, and relevant parse functions. Ensure new nodes carry line/column.
2) **Semantics**: Extend `TypeInfo`/`ClassInfo` if needed; add validation paths; update substitution/equality if generics-related; enforce null/override/visibility rules.
3) **Runtime**: Map new constructs into `RuntimeClass`/`RuntimeTypeInfo`; update layout/vtable construction; extend monomorphization if generics are involved.
4) **Error Surfaces**: Provide deterministic diagnostics (line/col). Keep runtime errors for impossible states minimal.
5) **Testing**: Add coverage in all layers touched. Minimum: parser + semantics; if runtime changes, add runtime/integration tests. Keep example files runnable.

## Common Pitfalls
- Forgetting to thread type arguments through cloning or substitution → runtime mismatches.
- Allowing `null` on arrays/primitives: must stay disallowed (ADR-002).
- Static + virtual/override: must remain an error.
- `super(...)` must stay first statement in constructors.
- Generic bases: remember to specialize base via `instantiateGeneric` when type args present.

## Adding New Features Safely
- Start with a small ADR/design note; align with existing constraints (single inheritance, monomorphization).
- Prefer additive changes; avoid weakening null/override rules without a new ADR.
- Keep `typeKey` stable if you extend RuntimeTypeInfo; otherwise update cache lookups and tests.

## References
- ADR-001 Class System (umbrella)
- ADR-002 Null Handling
- ADR-003 Class Generics
- ADR-004 @shots Annotation
- DES-001 Class System Architecture
- DES-002 @shots Annotation Design
