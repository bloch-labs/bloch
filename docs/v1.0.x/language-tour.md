---
title: Language Tour
---

Walk through core 1.0.x syntax, semantics, and patterns.

## Program shape
- `main` is required and is the entry point.
- Helper functions can be classical or marked `@quantum`.

```bloch
@quantum
function entangle(qubit a, qubit b) -> void {
    h(a);
    cx(a, b);
}

function main() -> void {
    @tracked qubit[2] q;
    entangle(q[0], q[1]);
    bit[2] out = {measure q[0], measure q[1]};
    echo(out);
}
```

## Types and declarations
- Scalars: `int`, `float`, `bit`, `char`, `string`, `qubit`.
- Arrays: `int[4]`, `float[]`, `bit[2]`, `qubit[2]`. Size expressions may be runtime values (validated at runtime).
- Constants: `final` makes a binding immutable after initialisation (`final int shots = 1024;`).
- Multiple qubit declarations are allowed: `qubit a, b, c;`.
- Strings use double quotes; chars use single quotes. Concatenate with `+` when either operand is a string: `"shots: " + shots`.

## Control flow and expressions
```bloch
function threshold(float value, float limit) -> bit {
    if (value > limit) {
        return 1;
    }
    return 0;
}
```
`if/else`, `for`, `while`, and ternary (`cond ? stmt1 : stmt2`) are available. Arithmetic, logical, and bitwise operators work on classical types. `bit[]` supports elementwise `& | ^ ~` with length checks.
- Common numeric patterns:
```bloch
function abs(int x) -> int {
    return (x < 0) ? -x : x;
}

function clamp(float v, float lo, float hi) -> float {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
```

## Measurements, reset, and tracking
- Measure inline: `bit b = measure q;`.
- Reset before reuse: `reset q;`.
- Track outcomes: add `@tracked` to the declaration and run with `--shots=N` to see aggregates.
- Classical control around measurements:
```bloch
function main() -> void {
    @tracked qubit q;
    h(q);
    bit r = measure q;
    if (r == 1) {
        z(q);
    }
    reset q;
}
```

```bloch
function main() -> void {
    @tracked qubit q;
    h(q);
    bit r = measure q;
    echo(r);
}
```

## Arrays and literals
```bloch
function dot(bit[2] a, bit[2] b) -> int {
    return a[0] * b[0] + a[1] * b[1];
}

function main() -> void {
    bit[2] lhs = {1, 0};
    bit[2] rhs = {1, 1};
    echo(dot(lhs, rhs)); // 1
}
```
Array literals validate element types at runtime. `qubit[]` must be declared, not initialised with literals.
- Dynamic sizing example:
```bloch
function zeros(int n) -> int[] {
    int[] v;
    // runtime-sized literal; length is enforced at runtime when used
    v = {0, 0, 0};
    return v;
}
```

## Strings, chars, and concatenation
- Strings use double quotes; chars use single quotes.
- Concatenate with `+` when either operand is a string: `"shots: " + shots`.
- `echo` accepts classical values; measure qubits first.

## Errors and diagnostics
- Parse errors: syntax problems (line/column included).
- Semantic errors: type/lifetime violations (e.g., re-measuring without reset, type mismatch, invalid assignment).
- Runtime errors: division by zero, out-of-bounds, invalid measurement target.
- Warnings: unmeasured qubits on scope exit (suppressed on intermediate shots when `--shots` > 1).

## Multi-shot execution and tracking
- `--shots=N` repeats the program N times.
- `@tracked` variables aggregate counts/probabilities; output prints after the final shot.
- Echo suppression: automatically disabled when many shots are taken unless you set `--echo=all`.

## Putting it together: Deutsch–Jozsa
```bloch
@quantum
function prepare(qubit[2] q) -> void {
    x(q[1]);
    h(q[0]); h(q[1]);
}

@quantum
function oracleBalanced(qubit data, qubit ancilla) -> void {
    cx(data, ancilla);
}

function runBalanced() -> bit {
    @tracked qubit[2] q;
    prepare(q);
    oracleBalanced(q[0], q[1]);
    h(q[0]);
    return measure q[0];
}

function main() -> void {
    bit result = runBalanced();
    echo(result); // expect 1 for the balanced oracle
}
```

Explore `examples/` in the repo for more patterns (Hadamard, Bell, Deutsch–Jozsa, Grover) and how to combine tracked registers with multi-shot runs.
