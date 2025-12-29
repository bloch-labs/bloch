---
title: Language Tour
---

This tour walks every construct available in Bloch 1.0.x with runnable examples and the physics behind them.

## Program shape
- A program is a single `.bloch` file with an entry `function main() -> void { ... }`.
- Functions are declared with explicit parameter and return types; there is no type inference.
- `@quantum` marks helper functions that contain quantum operations. `main` itself must stay classical (no `@quantum`).

## Values and literals
```bloch
int n = 4;                // decimal integers
float theta = 0.5f;       // float literals require a trailing f
bit flag = 1b;            // bit literals are 0b or 1b only
char c = 'a';             // single character, no escapes in 1.0.x
string msg = "hello";    // double-quoted strings, no escape sequences
qubit q;                  // qubit handle allocated at declaration
bit[4] outcomes;          // fixed-size array of bits
```
Arrays use brace literals:
```bloch
bit[2] basis = {0b, 1b};
float[3] angles = {0.0f, 0.5f, 1.0f};
```

## Variables, immutability, and tracking
```bloch
final int shots = 1024;   // immutable after assignment
@tracked qubit[2] qreg;   // record measurement histograms across shots
qubit[2] scratch;         // untracked; behaves normally but is not histogrammed
```
Use `@tracked` on qubits or qubit arrays when you intend to run multiple shots and inspect aggregated outcomes. Other types cannot be tracked in 1.0.x.

## Functions and `@quantum`
```bloch
@quantum
function applyHadamard(qubit q) -> void {
    h(q);
}

function main() -> void {
    qubit q;
    applyHadamard(q);
}
```
Rules:
- `@quantum` functions may only return `void`, `bit`, or `bit[]`.
- `main` cannot be annotated `@quantum`.
- Parameters and return types are mandatory; no overloading.

## Control flow
```bloch
function main() -> void {
    int iterations = 4;
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            echo("even " + i);
        } else {
            echo("odd " + i);
        }
    }

    int n = 0;
    while (n < 3) {
        echo(n);
        n++;
    }
}
```
`if/else`, `for`, `while`, and ternary statements (`cond ? then : else`) are available. `break`/`continue` are not in 1.0.x.

## Quantum operations
Built-in gates:
```bloch
h(q);              // Hadamard
x(q); y(q); z(q);  // Pauli
rx(q, 0.5f);       // rotations in radians
ry(q, -1.0f);
rz(q, 1.570796f);
cx(ctrl, tgt);     // controlled-X
```
Measurement and reset:
```bloch
bit b = measure q;
reset q;           // returns q to |0>, clears measured flag
```
In expressions, `measure q` yields a `bit`. As a statement, `measure q;` discards the result.

The expected Bell-state evolution is:
$$
|\psi\rangle = \frac{|00\rangle + |11\rangle}{\sqrt{2}} \quad\xrightarrow{\;\text{measure}\;}\quad \{00, 11\} \text{ with equal probability}.
$$

## Arrays of qubits and classical data
```bloch
qubit[3] q;
for (int i = 0; i < 3; i++) { h(q[i]); }
bit[3] results = {measure q[0], measure q[1], measure q[2]};
```
Array indexing is zero-based. Array sizes must be known at compile time; use `final` constants if you need derived lengths.

## Putting it together: GHZ(3)
```bloch
@quantum
function ghz3(qubit[3] q) -> void {
    h(q[0]);
    cx(q[0], q[1]);
    cx(q[1], q[2]);
}

function main() -> void {
    @tracked qubit[3] q;
    ghz3(q);
    bit[3] out = {measure q[0], measure q[1], measure q[2]};
    echo(out);
}
```
The circuit:
$$
\Qcircuit @C=1em @R=.7em {
  \lstick{|0\rangle} & \gate{H} & \ctrl{1} & \qw      & \qw      & \meter \\
  \lstick{|0\rangle} & \qw      & \targ   & \ctrl{1} & \qw      & \meter \\
  \lstick{|0\rangle} & \qw      & \qw     & \targ    & \qw      & \meter \\
}
$$
Run with `--shots=1024` to see concentration on `000` and `111`.

## Observability and QASM
- `echo` prints classical data; it is suppressed on all but the last shot unless `--echo=all` is provided.
- `--emit-qasm` writes OpenQASM alongside the source for interoperability.
