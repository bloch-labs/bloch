# Parser
Source: `src/bloch/compiler/parser/parser.hpp`, `src/bloch/compiler/parser/parser.cpp`

Inputs:
- `std::vector<Token>` from the lexer

Outputs:
- `std::unique_ptr<Program>` abstract syntax tree (AST)
- `BlochError` with `Parse` category on syntax errors.

Behaviour:
Parsing is a hand-written recursive descent pass with light lookahead for types and an overflow
queue to expand multi-declared qubits. It constructs the AST defined in `ast/ast.hpp` and stamps
every node with its source position. The parser enforces file-header ordering (`package` then
imports, both only at top level), supports both default-package and dotted imports, enforces
annotation placement, static-class restrictions, constructor/destructor syntax (including
`= default`), and rejects obviously invalid forms like negative array index literals. Deeper
meaning (types, visibility, override correctness) is left to the semantic analyser.

Invariants/Guarantees:
- Always consumes to `Eof` or throws; no partial trees.
- Multi-declared qubits (e.g., `qubit a, b;`) are expanded into separate declarations in order.

Edge Cases/Errors:
- Reports missing tokens (`expect`) with targeted messages (e.g., missing `;`, `)`).
- Rejects invalid annotations eagerly to avoid confusing downstream errors.

Extension points:
- Add productions by extending the relevant parse* method; keep expression precedence consistent with existing helpers.
- When adding syntax that introduces nodes, also extend AST definitions and semantic/runtime handling.

Tests:
- `tests/test_parser.cpp`

Related:
- `lexer.md`, `ast.md`, `semantics.md`
