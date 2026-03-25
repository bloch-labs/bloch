# Runtime Evaluator
Source: `src/bloch/runtime/runtime_evaluator.hpp`, `src/bloch/runtime/runtime_evaluator.cpp`

Inputs:
- Semantically valid AST `Program`
- CLI options (emit-qasm, shots, echo) forwarded by `src/bloch/cli/cli.cpp`

Outputs:
- Program side effects: echo output, tracked aggregation, QASM log
- Throws `BlochError` with `Runtime` category on runtime failures

Behaviour:
`RuntimeEvaluator` is a single-use interpreter. On `execute`, it builds runtime class metadata, initialises static fields, and starts a GC worker only when classes are present. It walks statements and expressions with an environment stack, class table, and heap, delegating quantum operations to the simulator and validating qubit usage (allocation, measurement, reset, and post-measurement guards). Tracked variables and fields record outcomes when scopes close, and multi-shot runs aggregate those counts. Constructors/destructors honour inheritance—`super(...)` chaining, default constructors binding parameters to fields, vtables for virtual dispatch, and base-first destructor execution unless skipped by the cycle collector. Echo output is buffered so warnings appear before user logs.

Invariants/Guarantees:
- Throws if reused (`execute` is single-use).
- Static field initialisers run once per class; instance field initialisers run during construction.
- Virtual dispatch uses vtable indices to honour overrides.
- QASM logging can be suppressed per evaluator instance.

Edge Cases/Errors:
- Out-of-bounds array indexing, invalid casts, or post-measurement gate usage raise `BlochError`.
- Cycle collector skips destructors for unreachable cycles without tracked fields and retains cycles with tracked fields (intentional trade-off).
- Runtime truthiness supports `boolean`, `bit`, `int`, `long`, and `float`, but semantic analysis restricts conditions to `boolean` or `bit`.
- Methods with non-void return types are not enforced to return a value (open issue).

Extension points:
- Add new value kinds: extend `Value::Type`, default initialisation, assignment/lookup semantics, and expression evaluation.
- Add new statements/expressions: extend `exec`/`eval` with line/column-aware errors.
- Integrate new backends: replace or wrap `QasmSimulator` while preserving the interface used here.

Tests:
- `tests/test_runtime.cpp`

Related:
- `qasm-simulator.md`, `semantics.md`, `module-loader.md`, `cli.md`
