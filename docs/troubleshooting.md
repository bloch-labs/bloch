# Troubleshooting

If you hit a roadblock, start with these quick checks:
- Pass a `.bloch` file as the last argument.
- Confirm imports point to existing files.
- Verify there is only one `main()` across all modules.
- Gate-after-measurement errors mean you need to reset or reallocate a qubit before applying
  more gates.
- `qubit[]` cannot be initialised with literals.
- Array indices in assignments must be `int` or `long` (other numeric indices are coerced at runtime when reading).
- Casting only supports `int`, `long`, `float`, and `bit`.

During multi-shot runs, echo defaults to off; pass `--echo=all` if you need output.

If problems persist, reduce shots to simplify debugging, add `echo` statements to inspect
state, and pay attention to line/column numbers in error messages. For deeper issues, revisit
these docs or the contributor references in `engineering/reference/`.
