---
title: Features
---

This page summarizes the v1.1.x language, runtime, class system, and tooling surface for production use.

## Language surface
- Strong, explicit typing: `int`, `float`, `bit`, `char`, `string`, `qubit`, and arrays of each (fixed or dynamic, e.g., `int[4]`, `float[]`, `qubit[2]`).
- Entry point: `function main() -> void` is mandatory.
- Annotations: `@quantum` for quantum helpers; `@tracked` on `qubit`/`qubit[]` for automatic measurement aggregation.
- Control: `if/else`, `for`, `while`, ternary; arithmetic, comparison, logical, and bitwise operators. `bit[]` supports elementwise `& | ^ ~` with length checks.
- Strings/chars: `+` concatenation works when either operand is `string`; `char` literals use single quotes.
- Imports: `import a.b;` to reference external modules (single-namespace in 1.1.x).

## Quantum model
- Gate set: `h`, `x`, `y`, `z`, `rx(q, theta)`, `ry(q, theta)`, `rz(q, theta)`, `cx(control, target)`.
- Measurement: expression form (`bit b = measure q;`) or inside array literals (`{measure q0, measure q1}`). Re-measuring without reset is an error.
- Reset: `reset q;` returns a qubit to `|0⟩` and clears its measured marker.
- Lifetime rules: leaving scope with unmeasured qubits raises warnings (suppressed on intermediate shots when `--shots` > 1). Measurements are tracked per qubit; `@tracked` aggregates at scope exit.

## Type system and semantics
- Immutable bindings: `final` creates an immutable variable after initialisation.
- Arrays: bounds-checked at runtime; negative indices are rejected at parse time. Literal initialisation is allowed for classical arrays; `qubit[]` must be declared without literals.
- Assignments: type-checked; invalid assignments (e.g., type mismatch, invalid target) are semantic errors.
- Errors are stratified: parse (syntax), semantic (type/lifetime), runtime (execution). Diagnostics include line/column.

## Class system
- Declarations: `class Name { ... }` with optional `extends Base`.
- Visibility: `public`/`private`/`protected` on fields and methods; defaults to `public`.
- Static: fields/methods may be `static`. Static classes are allowed (all members must be static).
- Inheritance: single inheritance via `extends`. `super` references the base; `override` enforces correct overriding of `virtual` base methods.
- Virtual/override: methods can be marked `virtual`; derived implementations must use `override` and match the signature/return type. Static methods cannot be virtual/override.
- Constructors/destructors: `constructor(...) -> ClassName` and `destructor -> void`. Constructors must return the class type. Destructors cannot take parameters and must return `void`.
- Methods/fields: standard declarations inside the class body. `this` refers to the current instance.
- Instantiation: `new ClassName(args)` produces an instance. Member access via `obj.field` and `obj.method(...)`. Static members are accessed via the class name (`ClassName.member`).
- Restrictions: single inheritance only; abstract methods require an implementation in derived non-abstract classes.
- Static classes cannot declare constructors/destructors or instance members.

## Execution model
- Interpreter backed by an ideal statevector simulator with an OpenQASM log.
- Hardware-agnostic design: programs remain portable to future backends.
- Multi-shot execution: `--shots=N` repeats runs; tracked variables emit counts/probabilities after the final shot. Echo output is buffered so warnings/errors appear first unless `--echo=all` is set.

## Tooling and CLI
- `bloch [options] <file.bloch>`
- Core options: `--shots=N`, `--emit-qasm`, `--echo=all|none`, `--update`, `--version`, `--help`.
- Outputs: `<file>.qasm` written alongside the source; tracked summaries when `--shots` > 1; echoed values for classical expressions.

## Diagnostics and safety
- Clear errors for invalid measurements (re-measure without reset), out-of-bounds indices, and type mismatches.
- Warnings for unmeasured qubits on scope exit; suppressed on intermediate multi-shot iterations to reduce noise.
- Runtime protects against division by zero and invalid array operations.

## Notes on imports and modules
- `import a.b;` is supported for modular layouts; resolution is single-namespace in 1.1.x. Keep module names aligned with file layout for clarity.
