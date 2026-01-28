# AST
Source: `src/bloch/core/ast/ast.hpp`

Inputs:
- Constructed by the parser from token streams

Outputs:
- Typed node tree consumed by semantics and runtime

Behaviour:
The AST layer is a catalogue of node structs covering expressions, statements, types, annotations, classes, functions, and the overall program. Each node stores its line and column, and every type supports `accept` so later passes can traverse uniformly via `ASTVisitor`.

Invariants/Guarantees:
- Ownership via `std::unique_ptr` for child nodes to avoid leaks and clarify lifetimes.
- Visitor must implement all node types; adding nodes requires extending the interface.

Edge Cases/Errors:
- None at the AST level; errors are thrown by consumers using node positions.

Extension points:
- Add new node structs and wire them into `ASTVisitor` plus inline `accept` helpers.
- Keep node fields minimal and source-driven; avoid storing derived data here (belongs in semantics/runtime).

Tests:
- `tests/test_ast.cpp`

Related:
- `parser.md`, `semantics.md`, `runtime-evaluator.md`
