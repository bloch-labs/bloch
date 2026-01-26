# Semantic Analyser
Source: `src/bloch/core/semantics/semantic_analyser.hpp`, `src/bloch/core/semantics/semantic_analyser.cpp`, `src/bloch/core/semantics/type_system.*`, `src/bloch/core/semantics/built_ins.*`

Inputs:
- AST `Program` from the parser

Outputs:
- Populated symbol tables and metadata (function signatures, class registry)
- Updated AST (array sizes resolved from const expressions)
- `BlochError` with `Semantic` category on failure

Behaviour:
The semantic analyser walks the AST with a scoped `SymbolTable`, recording declared names, their types, `final` status, and any const-int values used for array sizing. It validates declarations, visibility, static-versus-instance usage, and inheritance rules (single base, override/virtual correctness, abstract methods). Constructors and destructors are checked for default binding shape, parameter-field alignment, and uniqueness. Annotations are enforced—`@quantum` return types, `@shots` only on `main`, `@tracked` limited to variables and fields—and arrays must have compile-time, non-negative sizes with `qubit[]` barred from literal initialisation. Expression types are inferred to ensure assignments, returns, calls, casts, and member access are consistent. Built-in gate signatures are registered so runtime dispatch can trust the shapes.

Invariants/Guarantees:
- Every declared name is unique within scope; redeclarations fail.
- Class registry is built before analysis; missing bases or cycles error out.
- Functions are predeclared to allow call-before-definition.

Edge Cases/Errors:
- Invalid casts (e.g., to `char` or `void`) are rejected early.
- Methods currently do not enforce “must return a value” for non-void return types (open issue).
- Conditions in control flow are not type-checked to bit/int; runtime treats non-bit/int as false (open issue).

Extension points:
- Add new primitives: extend `ValueType`, `typeFromString/typeToString`, and inference paths.
- Add new annotations or rules: extend validation in relevant `visit` methods.
- Add new built-ins: extend `built_in_gates` and keep runtime/simulator in sync.

Tests:
- `tests/test_semantics.cpp`

Related:
- `ast.md`, `parser.md`, `runtime-evaluator.md`
