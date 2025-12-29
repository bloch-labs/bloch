---
title: Language Tour
---

A guided walkthrough of every construct available in Bloch 1.1.x, from primitives to classes and modules.

## Program layout
- One `main` function across all imported files. Decorate it with `@shots(N)` to control shot count from source.
- Modules are plain `.bloch` files; use `import path.to.module;` to merge them. Import cycles are rejected.

## Values and literals
```bloch
int n = 4;
float theta = 0.5f;        // floats require trailing f
bit flag = 1b;             // 0b or 1b only
char c = 'x';              // no escape sequences
string msg = "hello";     // double-quoted, raw characters
qubit q;                   // allocates |0>
bit[2] pair = {0b, 1b};    // arrays have fixed size
```

## Functions and `@quantum`
```bloch
@quantum
function rotate(qubit q, float theta) -> void {
    ry(q, theta);
}

function main() -> void {
    qubit q;
    rotate(q, 0.5f);
}
```
Rules mirror 1.0.x: explicit types, `@quantum` functions must return `void`/`bit`/`bit[]`, and `main` cannot be `@quantum`.

## Control flow
`if/else`, `for`, `while`, and ternary conditionals are available. `break`/`continue` are not present.

## Quantum operations
Built-ins:
```bloch
h(q);
rx(q, 0.25f);
rz(q, -1.570796f);
cx(ctrl, tgt);
bit b = measure q;
reset q;
```
Measurement collapses the qubit and produces a `bit`; reset returns it to \(|0\rangle\).

The Bell pair uses the same circuit as 1.0.x:
$$
\Qcircuit @C=1em @R=.7em {
  \lstick{|0\rangle} & \gate{H} & \ctrl{1} & \meter \\
  \lstick{|0\rangle} & \qw      & \targ   & \meter \\
}
$$

## Classes
Classes encapsulate state and methods with visibility and inheritance.
```bloch
class Accumulator {
    private int total = 0;

    public constructor() -> Accumulator = default;

    public function add(int v) -> void {
        total = total + v;
    }

    public function value() -> int {
        return total;
    }
}

function main() -> void {
    Accumulator acc = new Accumulator();
    acc.add(2);
    echo(acc.value());
}
```

### Constructors and destructors
- `constructor(...) -> ClassName { ... }` runs on `new ClassName(...)`.
- `= default` constructors map parameters to non-qubit, non-static fields of the same name and type.
- A class may declare at most one `destructor() -> void` (or `= default`). Call it early with `destroy obj;`; otherwise it runs when the object is collected.

### Visibility and modifiers
- Members default to `private` in instance classes and `public` in `static class`.
- Use `public` / `private` / `protected` on fields and methods.
- `static` members belong to the type; `static class` forbids constructors and instance fields.
- `virtual` marks a method as overridable; `override` enforces signature match against the base.

### Inheritance and dispatch
```bloch
abstract class Layer {
    virtual function apply(qubit q) -> void;
}

class HadamardLayer extends Layer {
    public override function apply(qubit q) -> void { h(q); }
}
```
- Single inheritance via `extends Base`.
- `super` is only valid inside constructors to chain into a base constructor.
- `this` refers to the current instance; instance members are inaccessible from static contexts.

### Quantum methods
Methods may be annotated `@quantum` and follow the same return restrictions as functions. They participate in dispatch normally (virtual/override allowed) and must respect static-context rules.

### Object lifecycle
- `destroy obj;` runs the destructor immediately and marks the object as destroyed.
- Tracked qubit fields retain tracking data across shots; non-tracked fields are cleaned up automatically.

## Tracking and shots
- Annotate qubits or qubit fields with `@tracked` to gather histograms.
- Use `@shots(N)` on `main` for multi-shot runs. CLI `--shots` remains for compatibility but defers to the annotation when both are present (and warns if they differ).
- Echo output defaults to the final shot; use `--echo=all` to print per shot.

## Modules and imports
```bloch
// lib/angles.bloch
function rzpi(qubit q) -> void { rz(q, 3.141592f); }

// app.bloch
import lib.angles;

function main() -> void {
    qubit q;
    rzpi(q);
}
```
Resolution order: importing file’s directory, configured search paths, then current working directory. Only one `main` may exist across all imported modules.

## Putting it together: class-based GHZ
```bloch
class GhzBuilder {
    public constructor() -> GhzBuilder = default;

    @quantum
    public function build(qubit[] q) -> void {
        h(q[0]);
        for (int i = 1; i < 3; i++) { cx(q[i-1], q[i]); }
    }
}

@shots(1024)
function main() -> void {
    @tracked qubit[3] q;
    GhzBuilder gb = new GhzBuilder();
    gb.build(q);
    bit[3] out = {measure q[0], measure q[1], measure q[2]};
    echo(out);
}
```
