# Codebase Overview

This document gives a high-level map of the Bloch codebase and the execution pipeline.

## High-level flow
1. **Module loader** resolves imports and merges modules.
2. **Lexer** tokenizes source and tracks line/column.
3. **Parser** builds the AST.
4. **Semantic analysis** validates types, annotations, and class rules.
5. **Runtime evaluator** interprets the AST and drives the quantum simulator.
6. **QASM output** is emitted alongside the input file.

## Repository layout
- `src/bloch/compiler/` — lexer, parser, AST, semantic analysis, imports.
- `src/bloch/cli/` — CLI entry and option parsing.
- `src/bloch/http/` — HTTP helpers (used by update flows).
- `src/bloch/runtime/` — interpreter, runtime values, simulator.
- `src/bloch/support/` — diagnostics, logging, shared utilities.
- `src/bloch/update/` — self-update and version checks.
- `tests/` — unit, semantic, runtime, and integration tests.
- `examples/` — runnable Bloch programs.
- `docs/` — user documentation.
- `engineering/` — contributor docs, ADRs, and designs.

## Key entry points
- `src/main.cpp` — Thin process entry point that forwards to CLI runtime.
- `src/bloch/cli/cli.*` — CLI parsing, update checks, stdlib path resolution, and execution flow.
- `src/bloch/compiler/import/module_loader.*` — import resolution and merge.
- `src/bloch/compiler/semantics/semantic_analyser.*` — compile-time rules.
- `src/bloch/runtime/runtime_evaluator.*` — interpreter and simulator integration.

## Further reading
- [Architecture](./architecture.md)
- [Parser](./parser.md)
- [Semantics](./semantics.md)
- [Runtime Evaluator](./runtime-evaluator.md)
