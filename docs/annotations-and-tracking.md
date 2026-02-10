# Annotations & Tracking

Annotations communicate intent to the compiler and runtime.

- `@quantum` marks functions or methods that run quantum code; they must return `bit`, `bit[]`,
  or `void`, and `@quantum` may not decorate `main()`.
- `@shots(N)` applies to `main()` to run the program `N` times and aggregate tracked values.
- `@tracked` on `qubit` primitives records measurement outcomes.

Tracked values behave predictably:
- A `@tracked qubit` records the last measurement (`0`, `1`, or `?` if unmeasured).
- A `@tracked qubit[]` records a bitstring if all elements were measured, otherwise `?`.
- Local variables are recorded at scope exit.
- Tracked fields are recorded when the object is destroyed (explicitly or at program shutdown).

Only `qubit` and `qubit[]` values produce tracked output; other types are ignored.

Example:
```bloch
@shots(1024)
function main() -> void {
    @tracked qubit q;
    h(q);
    measure q;
}
```

Echo prints during execution; tracking happens after execution. Control echo volume with
`--echo=all|none|auto`.
