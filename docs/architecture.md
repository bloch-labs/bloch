# Bloch Architecture

This document is a technical map for contributors who want to understand how the Bloch compiler and runtime fit together. It mirrors the layout of `src/` and calls out the most important extension points.

## High-Level Flow

```
.bloch source ──► Lexer ──► Tokens ──► Parser ──► AST
                                   │
                                   ▼
                           Semantic Analyser ──► Validated AST
                                   │
                                   ▼
                            Runtime Evaluator ──► QASM + Simulation
```

1. **Lexing** (`src/bloch/lexer`) turns raw source into a flat token stream.
2. **Parsing** (`src/bloch/parser`) converts tokens into the tree defined in `src/bloch/ast/ast.hpp`.
3. **Semantic analysis** (`src/bloch/semantics`) resolves symbol tables, enforces type rules, checks annotations, and builds `FunctionInfo` metadata used later by the runtime.
4. **Runtime evaluation** (`src/bloch/runtime`) walks the AST, simulates quantum behaviour via a lightweight QASM backend, and tracks `@tracked` statistics.
5. **CLI** (`src/main.cpp`) wires all stages together and exposes command-line UX.

Tests in `tests/` exercise each layer with a minimal custom harness (`tests/test_framework.hpp`).

## Modules

### Lexer (`src/bloch/lexer`)
- Responsible for whitespace, comment handling, identifiers, literals, and multi-character operators.
- Emits `Token` objects tagged with type and source location.
- Minimal state: current cursor, line and column counters.

### Parser (`src/bloch/parser`)
- Implements a recursive-descent parser over the token stream.
- Allocates AST nodes using `std::unique_ptr`, guaranteeing tree ownership.
- Reports `BlochError` with location metadata when syntax issues arise.

### AST (`src/bloch/ast`)
- Contains node definitions and the base `ASTVisitor` interface used by later phases.
- Keeps nodes lean: they store only data needed by passes and defer heavy lifting (e.g. type names as strings) to the semantic layer.

### Semantics (`src/bloch/semantics`)
- `SemanticAnalyser` walks the AST with scoped symbol tables (`SymbolTable` + `ValueType` enum).
- Validates: redeclarations, typing rules, `@tracked` restrictions, function contracts, built-in gate usage.
- Stores function signatures in `m_functionInfo` for runtime validation.
- Recent change: scopes are now managed with RAII (`std::scope_exit`) to guarantee cleanup on exceptions.

### Runtime (`src/bloch/runtime`)
- `RuntimeEvaluator` interprets the validated AST, managing variable environments, qubit lifetimes, tracked counters, and measurement history.
- Delegates low-level simulation to `QasmSimulator` (emits textual QASM for interoperability).
- Value storage relies on the `Value` struct; primitives and arrays are represented separately to keep memory tight and avoid allocations in hot paths.
- Scope lifetimes are also guarded with `std::scope_exit` to prevent leaks when execution aborts early.

### CLI (`src/main.cpp`)
- Handles argument parsing, file IO, and multi-shot execution logic (`--shots`, `--emit-qasm`, `--echo` flags).
- Persists QASM output alongside the source file and composes tracked statistics per measurement.

## Directory Reference

| Path | Purpose |
| ---- | ------- |
| `src/bloch/lexer` | Tokenisation layer |
| `src/bloch/parser` | Syntax tree construction |
| `src/bloch/ast` | Shared node definitions |
| `src/bloch/semantics` | Type system, built-ins, semantic checks |
| `src/bloch/runtime` | Interpreter, simulator integration |
| `scripts/` | Tooling (formatting, install, release helpers) |
| `tests/` | Unit + integration suites using the internal test harness |

## Extending the Language

1. **Grammar changes** – update the parser and AST nodes; keep node ownership with `std::unique_ptr`.
2. **Semantic rules** – extend `SemanticAnalyser` to validate the new syntax and update the type system helpers in `type_system.cpp`.
3. **Runtime behavior** – update `RuntimeEvaluator` and, if needed, the simulator or tracked counters.
4. **Tests** – add coverage to the targeted suite (lexer/parser/semantics/runtime) to keep CI coverage above 90%.

When in doubt, sketch the change alongside this flow: token → AST → semantic → runtime. Keeping modules self-contained makes it easier for contributors to reason about regressions.

## Future Enhancements (Ideas)

- Move runtime values to `std::variant` for stronger type safety.
- Introduce an `include/` surface exposing a stable API for editor integrations.
- Provide a visitor helper library to replace long `dynamic_cast` chains in the semantic pass.
- Explore replacing the custom test harness with Catch2 or GoogleTest once the ergonomics justify the dependency.

These will be expanded on the project wiki; feel free to open an issue if you want to help drive any item.
