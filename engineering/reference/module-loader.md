# Module Loader
Source: `src/bloch/compiler/import/module_loader.hpp`, `src/bloch/compiler/import/module_loader.cpp`

Inputs:
- Entry file path passed from the CLI
- Optional search paths (constructor argument)

Outputs:
- Merged `Program` containing all classes/functions/statements from entry + imports
- `BlochError` with `Semantic`/`Parse` category on missing modules or cycles

Behaviour:
The loader resolves dotted imports against the importing file first, then any configured search paths, and finally the working directory. It keeps a stack to detect cycles and surfaces the full chain when one appears. Each module is parsed only once and cached; after loading, classes, functions, and statements are merged in load order into a single `Program`. It enforces that only one `main()` exists across the merged graph, captures any `@shots` decoration on `main`, and strips import nodes before handing control to later phases.

Invariants/Guarantees:
- Each module is parsed at most once per load invocation.
- If a module is missing or a cycle exists, loading aborts with an error; no partial programs are returned.

Edge Cases/Errors:
- Multiple `main()` definitions across modules error out.
- Unresolved import paths include the qualified name in diagnostics.

Tests:
- `tests/test_runtime.cpp`

Related:
- `parser.md`, `semantics.md`, `runtime-evaluator.md`, `cli.md`
