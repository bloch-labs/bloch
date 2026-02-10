# Troubleshooting

If you hit a roadblock, start with the quick checks below and the [Common issues](#common-issues) table. For error message format and examples, see [Errors & Diagnostics](./diagnostics.md).

## Quick checks

- Pass a `.bloch` file as the last argument to `bloch`.
- Confirm imports point to existing files (e.g. `import QuantumMath;` expects `QuantumMath.bloch` in the expected path).
- Verify there is only one `main()` across all modules.
- Gate-after-measurement errors mean you need to `reset` or reallocate a qubit before applying more gates.
- `qubit[]` cannot be initialised with literals.
- Array indices in assignments must be `int` or `long` (other numeric indices are coerced at runtime when reading).
- Casting only supports `int`, `long`, `float`, and `bit` — not `boolean`, `char`, `string`, or class types.

During multi-shot runs, echo defaults to off; pass `--echo=all` if you need output.

If problems persist, reduce shots to simplify debugging, add `echo` statements to inspect state, and pay attention to line/column numbers in error messages. If you have found a bug please raise an issue on GitHub (https://github.com/bloch-labs/bloch) or email hello@bloch-labs.com.

## Common issues

| Symptom | Cause | Fix |
|--------|--------|-----|
| "expected .bloch file" or wrong file run | No file passed or wrong path | Pass the path to your `.bloch` file as the last argument: `bloch path/to/file.bloch`. |
| Import / module not found | Import path doesn’t resolve to a file | Ensure the imported module exists (e.g. `QuantumMath.bloch` for `import QuantumMath;`) and the loader can find it (e.g. same directory or configured path). |
| Duplicate definition of main | More than one `main()` in the program | Keep exactly one `main()` across all files that are imported or run. |
| Gate-after-measurement error | Gate applied to a qubit that was already measured | Call `reset q;` before applying more gates to `q`, or use a new qubit. See [Quantum Programming](./quantum-programming.md). |
| `qubit[]` initialisation error | Literal used for qubit array | `qubit[]` cannot be initialised with `{ ... }`; declare and use qubits individually or with `qubit[N] name;`. |
| Wrong type in assignment | Mismatched types (e.g. string to int) | Use the correct type or an explicit cast. Only `int`, `long`, `float`, `bit` casts are supported. See [Casting](./casting.md). |
| Array index in assignment rejected | Index not `int` or `long` at compile time | Use an `int` or `long` expression as the index in assignments; other numerics are allowed only when reading. |
| No output when using many shots | Echo suppressed by default | Use `--echo=all` to see per-shot output. |

## Get help

- **Documentation:** Revisit the guides linked from the [docs README](./README.md), especially [Errors & Diagnostics](./diagnostics.md) and the [Language Specification](./language-spec.md).
- **Community and support:** See the main repository [README](../README.md) for the project website, community links, and contact (e.g. GitHub Discussions, email).
- **Contributor / implementation details:** For deep dives into the compiler and runtime, see the contributor references in `engineering/reference/`.
