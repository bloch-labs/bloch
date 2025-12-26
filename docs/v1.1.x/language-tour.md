---
title: Language Tour
---

Walk through Bloch 1.1.x syntax, semantics, patterns, and best practices.

## Program structure
- `main` is required and is the entry point.
- Helpers can be classical or marked `@quantum`.
- Keep quantum helpers small; orchestrate with classical control for readability.

```bloch
@quantum
function prepareBell(qubit a, qubit b) -> void {
    h(a);
    cx(a, b);
}

function main() -> void {
    @tracked qubit[2] q;
    prepareBell(q[0], q[1]);
    bit[2] out = {measure q[0], measure q[1]};
    echo(out);
}
```

## Types and declarations
- Scalars: `int`, `float`, `bit`, `char`, `string`, `qubit`.
- Arrays: sized or dynamic (`int[4]`, `float[]`, `bit[2]`, `qubit[2]`). Size expressions are validated at runtime when dynamic.
- Constants: `final` makes a binding immutable after initialisation (`final int shots = 1024;`).
- Multiple qubit declarations are supported: `qubit a, b, c;` (no initialisers allowed in a multi-declaration).
- Strings use double quotes; chars use single quotes. Concatenate with `+` when either operand is a string: `"shots: " + shots`.

## Annotations
- `@quantum` on functions isolates quantum logic (future placement/analysis uses this).
- `@tracked` on `qubit`/`qubit[]` aggregates measurements at scope exit and prints results on multi-shot runs.

## Control flow and expressions
```bloch
function parity(bit[4] bits) -> bit {
    int acc = 0;
    for (int i = 0; i < 4; i = i + 1) {
        acc = acc + bits[i];
    }
    return acc % 2;
}
```
- Conditionals: `if/else`, ternary (`cond ? stmt1 : stmt2`).
- Loops: `for (init; cond; update) { ... }`, `while (cond) { ... }`.
- Operators: arithmetic (`+ - * / %`), comparisons, logical (`&& || !`), bitwise (`& | ^ ~`). `bit[]` supports elementwise bitwise ops with size checks.
- Assignment targets can be variables, array indices, or struct members (when class runtime support lands).
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

## Quantum operations
- Gates: `h`, `x`, `y`, `z`, `rx(qubit, theta)`, `ry(qubit, theta)`, `rz(qubit, theta)`, `cx(control, target)`.
- Measurement: expression form `bit b = measure q;` or inside literals `{measure q0, measure q1}`. Re-measuring a qubit without reset raises an error.
- Reset: `reset q;` returns a qubit to `|0⟩` and clears its measured marker.
- Lifetime: leaving scope with unmeasured qubits triggers warnings (suppressed on intermediate shots when `--shots` > 1).
- Classical control around quantum operations:
```bloch
function main() -> void {
    @tracked qubit q;
    int flips = 3;
    if (flips % 2 == 1) {
        x(q);
    }
    bit b = measure q;
    echo(b);
}
```

## Arrays and literals
```bloch
function sum(int[] xs) -> int {
    int total = 0;
    for (int i = 0; i < 4; i = i + 1) {
        total = total + xs[i];
    }
    return total;
}

function main() -> void {
    int[4] values = {1, 1, 2, 3};
    echo(sum(values)); // 7
}
```
- Classical array literals validate element types at runtime.
- `qubit[]` must be declared, not initialised via literals.
- Indexing is bounds-checked; negative indices are rejected at parse time.
- Dynamic sizing example:
```bloch
function fill(int n) -> int[] {
    int[] out;
    // runtime-sized array via literal; size is validated when used
    out = {0, 0, 0};
    return out;
}
```

## Strings, chars, and concatenation
- Strings use double quotes; chars use single quotes.
- Concatenate with `+` when either operand is a string: `"shots: " + shots`.
- `echo` accepts any classical value; qubits must be measured first.

## Error handling and diagnostics
- Parse errors: syntax problems (line/column included).
- Semantic errors: type/lifetime violations (e.g., re-measuring, type mismatch, invalid assignment).
- Runtime errors: division by zero, out-of-bounds, invalid measurement target.
- Warnings: unmeasured qubits on scope exit (unless suppressed during multi-shot aggregation).

## Multi-shot execution and tracking
- `--shots=N` repeats the program N times.
- `@tracked` variables aggregate counts/probabilities; output prints after the final shot.
- Echo suppression: automatically disabled when many shots are taken unless you set `--echo=all`.

## Pattern: measurement feedback and reuse
```bloch
function main() -> void {
    @tracked qubit q;
    h(q);
    bit result = measure q;
    if (result == 1) {
        z(q);           // correct phase if desired
    }
    reset q;            // clear measured marker for reuse
}
```

## Classes and objects
```bloch
class Counter {
    private int value;

    constructor(int start) -> Counter {
        value = start;
        return this;
    }

    function inc() -> void {
        value = value + 1;
    }

    function get() -> int {
        return value;
    }
}

class QuantumBox extends Counter {
    @quantum
    function apply(qubit q) -> void {
        h(q);
    }

    override function inc() -> void {
        super.inc();
        // could log or adjust
    }
}

function main() -> void {
    Counter c = new Counter(0);
    c.inc();
    echo(c.get()); // 1

    @tracked qubit q;
    QuantumBox qb = new QuantumBox(0);
    qb.apply(q);
    bit b = measure q;
    echo(b);
}
```
- Single inheritance via `extends`.
- `constructor` must return the class type; `destructor -> void` (not shown) has no parameters.
- Visibility defaults to `public`; use `private`/`protected` for encapsulation.
- Static fields/methods are accessed via the class name: `ClassName.member`.

## Preview: imports and classes (1.1.x)
- Syntax exists for `import`, classes, visibility (`public/private/protected`), `static`, `virtual`, `override`, constructors/destructors.
- Parsing and type-checking are present; runtime execution for classes will land during the 1.1.x series. Use functional style in production until then.

## Putting it together: Grover on 2 qubits
```bloch
@quantum
function oracle11(qubit[2] q) -> void {
    h(q[1]);
    cx(q[0], q[1]);
    h(q[1]);
}

@quantum
function diffusion(qubit[2] q) -> void {
    h(q[0]); h(q[1]);
    x(q[0]); x(q[1]);
    // conditional phase flip
    h(q[1]); cx(q[0], q[1]); h(q[1]);
    x(q[0]); x(q[1]);
    h(q[0]); h(q[1]);
}

function main() -> void {
    @tracked qubit[2] data;
    h(data[0]); h(data[1]);
    oracle11(data);
    diffusion(data);
    bit[2] out = {measure data[0], measure data[1]};
    echo(out); // ~100% "11" over many shots
}
```

See the `examples/` directory for more patterns (Hadamard, Bell, Deutsch–Jozsa, Grover) and best practices for tracked registers and multi-shot runs.
