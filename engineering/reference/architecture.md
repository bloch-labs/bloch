# Architecture

The Bloch toolchain flows from the CLI down to the simulator in a straight line. `src/main.cpp`
is a thin wrapper that delegates to `src/bloch/cli/cli.cpp`, where flags are parsed, update checks
run, and stdlib search paths are prepared. The module loader resolves imports relative to the
caller and optional search paths, auto-loads `bloch.lang.Object` when available, and merges
everything into a single program while enforcing that only one `main()` exists. The lexer, parser,
and AST builder convert source into a typed tree with source locations; the semantic analyser then
checks names, types, annotations, visibility, inheritance rules, and built-in gate signatures.
Only if semantics succeed does the runtime step in: `RuntimeEvaluator` walks the AST, constructs
runtime class metadata, executes classical code, and delegates quantum operations to the
`QasmSimulator`. The simulator maintains the statevector, prevents gates on measured qubits, and
logs OpenQASM that is written beside the source file after execution. Tracked variables aggregate
measurement outcomes across shots when requested, and errors anywhere in the chain surface as
`BlochError` with line/column when available. Extending the system means threading changes through
the same pipeline: new syntax or gates begin in the front-end, pick up validation in semantics,
and finish with runtime behaviour and simulator support, while tests in `tests/` keep the flow
stable.
