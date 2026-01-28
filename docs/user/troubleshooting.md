# Troubleshooting

If you hit a roadblock, start with these quick checks: ensure you pass a `.bloch` file as the last argument, confirm imports point to existing files, and verify there is only one `main()` across all modules. Gate-after-measurement errors mean you need to reset or reallocate a qubit before applying more gates. `qubit[]` cannot be initialised with literals, and only casts to `int`, `float`, or `bit` are allowed. During multi-shot runs, echo defaults to off; pass `--echo=all` if you need output.

If problems persist, reduce shots to simplify debugging, sprinkle `echo` statements to inspect state, and pay attention to the line/column in error messages—they map directly to your source. For deeper issues, revisit the user docs or the contributor references in `docs/reference/*`.
