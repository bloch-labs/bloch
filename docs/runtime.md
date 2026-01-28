---
title: Bloch Runtime Overview
---
# Bloch Runtime Overview

Bloch executes programs with a tree-walking interpreter and an ideal statevector simulator that logs OpenQASM 2.0 operations.

## Architecture

1. Lexer: Produces tokens with line/column info; skips whitespace and `//` comments.
2. Parser: Builds the AST following the [Grammar](./grammar).
3. Semantic Analysis: Validates scopes, `final`, function contracts, built-in calls, `@tracked`, and return rules (see [Semantics](./language/semantics)).
4. Runtime Evaluator: Interprets statements/expressions, buffers `echo()` output, calls the simulator for gates, and records measurements.

## Simulator

`QasmSimulator` maintains a statevector and emits a QASM log. Gates update amplitudes; `measure` collapses and writes `measure q[i] -> c[i];` to the log. `reset` sends a qubit to `|0>` robustly.

## QASM emission

The CLI always writes `<file>.qasm` next to your source. Use `--emit-qasm` to also print it to stdout.

## Key Files

- `src/bloch/lexer/` – lexical analysis
- `src/bloch/parser/` – recursive descent parser
- `src/bloch/ast/` – AST nodes
- `src/bloch/semantics/` – semantic analyser
- `src/bloch/runtime/` – interpreter and simulator
