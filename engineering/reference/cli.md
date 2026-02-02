# CLI Entry
Source: `src/main.cpp`, `src/bloch/cli/cli.*`

Inputs:
- CLI arguments: `--help`, `--version`, `--update`, `--emit-qasm`, `--shots=N` (to be deprecated in v2.0.0), `--echo=auto|all|none`, `<file.bloch>`
- Build-time defines: `BLOCH_VERSION`, `BLOCH_COMMIT_HASH`

Outputs:
- Process exit code (0 on success)
- Echo/tracked output to stdout/stderr
- `<file>.qasm` written alongside the source
- Background update check (stderr notice on availability)

Behaviour:
The CLI parses flags, validates shot and echo options, and warns when `--shots=N` conflicts with `@shots`. It runs a non-blocking update check on startup and can self-update when requested. After parsing, it invokes the module loader, semantic analyser, and runtime interpreter to execute the program. Multi-shot mode reruns the interpreter, aggregates tracked values, prints per-variable summaries, and preserves the last shot’s QASM. Echo defaults to `auto`, suppressing output during large shot counts unless explicitly overridden.

Invariants/Guarantees:
- Requires a single input file; errors out with usage on missing file.
- Always writes QASM, even when `--emit-qasm` is off (file only).
- Exits early after `--help`/`--version`/`--update`.

Edge Cases/Errors:
- Invalid `--shots` (non-positive) or mismatched `--shots` vs. `@shots` produce warnings/errors as applicable.
- Exceptions from lower layers are caught and printed with a “Stopping program execution...” prefix.

Extension points:
- Add flags by extending `kCliOptions` and the argument loop; ensure semantics/runtime support the feature.

Tests:
- `tests/test_main.cpp` and `tests/test_integration.cpp`

Related:
- `update-manager.md`, `module-loader.md`, `runtime-evaluator.md`
