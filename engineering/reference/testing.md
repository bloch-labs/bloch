# Testing
Source: `tests/*`, `scripts/build_and_test.sh`

Inputs:
- Built test binaries via CMake

Outputs:
- Pass/fail results; diagnostics from assertions

Behaviour:
The suite is split into focused unit tests and integration runs. Unit suites cover lexing, parsing, semantics, runtime, and AST shapes through files like `tests/test_lexer.cpp`, `tests/test_parser.cpp`, `tests/test_semantics.cpp`, `tests/test_runtime.cpp`, and `tests/test_ast.cpp`. Integration coverage in `tests/test_integration.cpp` and `tests/test_main.cpp` exercises CLI and end-to-end flows. All tests share the lightweight harness in `tests/test_framework.hpp`, and you can run them with `ctest --test-dir build --output-on-failure` or via `scripts/build_and_test.sh`.

Invariants/Guarantees:
- Tests assume deterministic simulator RNG seed from `std::random_device`; quantum randomness is still probabilistic, so assertions avoid brittle probability checks.

Edge Cases/Errors:
- Tests that expect errors often assert on `BlochError` line/column; keep those stable when editing parser/semantics.

Extension points:
- Add cases to existing suites rather than creating new files to keep coverage discoverable.
- For new language/runtime features, add coverage across relevant layers (lexer/parser/semantics/runtime).

Related:
- `coding-standards.md`
