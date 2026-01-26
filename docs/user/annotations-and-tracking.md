# Annotations & Tracking

Annotations communicate intent to the compiler and runtime. Use `@quantum` on functions or methods that run quantum code; they must return `bit`, `bit[]`, or `void`. Apply `@shots(N)` to `main()` to run the program `N` times and aggregate tracked values. Add `@tracked` to variables or fields to record measurement outcomes automatically when their scope ends.

Tracked values behave predictably: a `@tracked qubit` records the last measurement (`0`, `1`, or `?` if unmeasured), and a `@tracked qubit[]` records a bitstring if all elements were measured, otherwise `?`. Aggregation happens across shots when `@shots(N)` or `--shots=N` (deprecated) is used.

Example:
```bloch
function main() -> void {
    @tracked qubit q;
    h(q);
    measure q;
}
```

Echo prints during execution; tracking happens after execution. Control echo volume with `--echo=all|none|auto`. Note that `@quantum` cannot decorate `main()`, and `@shots` only makes sense on `main()`.
