# Bloch Runtime Model

Bloch executes programs with a tree-walking interpreter and an ideal statevector simulator that
logs OpenQASM 2.0 operations.

## Execution pipeline
1. Lexer: produces tokens with line/column info; skips whitespace and `//` comments.
2. Parser: builds the AST following the [Grammar](./grammar.md).
3. Semantic analysis: validates scopes, types, annotations, and class rules (see the
   [Language Specification](./language-spec.md)).
4. Runtime evaluator: interprets statements/expressions, buffers `echo()` output, calls the
   simulator for gates, and records measurements.

## Simulator and QASM
`QasmSimulator` maintains a statevector and emits a QASM log. Gates update amplitudes;
`measure` collapses and writes `measure q[i] -> c[i];` to the log. `reset` sends a qubit to
`|0>`.

## QASM emission
The CLI always writes `<file>.qasm` next to your source. Use `--emit-qasm` to also print it to
stdout.

## Key locations
- `src/bloch/compiler/lexer/` – lexical analysis
- `src/bloch/compiler/parser/` – recursive descent parser
- `src/bloch/compiler/semantics/` – semantic analyser
- `src/bloch/runtime/` – interpreter and simulator
