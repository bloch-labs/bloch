---
title: Classes
---

Comprehensive guide to the Bloch 1.1.x class system: declarations, inheritance, visibility, static members, constructors/destructors, virtual/override, and usage patterns.

## Declaring classes
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
```
- `class Name { ... }` declares a class. Single inheritance uses `extends Base`.
- Default visibility is `public` if unspecified on members.
- Fields and methods are declared inside the class body.

## Visibility
- `public` — accessible from anywhere.
- `private` — accessible only inside the declaring class.
- `protected` — accessible inside the class and its derived classes.

## Static members
```bloch
class Globals {
    public static int version = 1;
    public static function bump() -> void { version = version + 1; }
}

function main() -> void {
    Globals.bump();
    echo(Globals.version); // 2
}
```
- Static fields/methods belong to the class, not instances.
- Static classes are allowed; all members must be static. Static classes cannot declare constructors, destructors, or instance members.
- Access static members via `ClassName.member`.

## Constructors and destructors
```bloch
class Accumulator {
    private int total;

    constructor(int start) -> Accumulator {
        total = start;
        return this;
    }

    destructor -> void {
        // clean up if needed
    }
}
```
- Constructors:
  - Signature: `constructor(params) -> ClassName { ... }`
  - Must return the class type.
  - Cannot be `static`, `virtual`, or `override`.
  - Not allowed in static classes.
- Destructors:
  - Signature: `destructor -> void { ... }` (no parameters).
  - Cannot be `static`, `virtual`, or `override`.
- Instantiation: `Accumulator a = new Accumulator(0);`

## Inheritance, virtual, and override
```bloch
class Base {
    virtual function describe() -> string {
        return "base";
    }
}

class Derived extends Base {
    override function describe() -> string {
        return "derived";
    }
}
```
- Single inheritance only (`extends Base`).
- `virtual` marks a method that may be overridden.
- `override` must match the base `virtual` signature and return type; static methods cannot be virtual/override.
- Use `super.method()` to call the base implementation from an override.

## Abstract methods and classes
```bloch
class Shape {
    virtual function area() -> float; // abstract (no body)
}

class Square extends Shape {
    private float side;
    constructor(float s) -> Square { side = s; return this; }
    override function area() -> float { return side * side; }
}
```
- A `virtual` method without a body is abstract.
- Any non-abstract subclass must implement all abstract base methods with `override`.
- Abstract classes cannot be instantiated until all abstract methods are implemented.

## Member access and `this` / `super`
```bloch
class Logger {
    protected string name;
    constructor(string n) -> Logger { name = n; return this; }
    virtual function id() -> string { return name; }
}

class PrefixedLogger extends Logger {
    constructor(string n) -> PrefixedLogger { super(n); return this; }
    override function id() -> string { return "log:" + super.id(); }
}
```
- `this` refers to the current instance.
- `super(...)` calls the base constructor; `super.method()` calls a base method.
- Accessing `super` members follows visibility rules.

## Example: quantum + class orchestration
```bloch
class QuantumBox {
    constructor() -> QuantumBox { return this; }

    @quantum
    function apply(qubit q) -> void {
        h(q);
    }
}

function main() -> void {
    @tracked qubit q;
    QuantumBox qb = new QuantumBox();
    qb.apply(q);
    bit b = measure q;
    echo(b);
}
```
- Quantum methods follow the same visibility rules.
- Classes can host quantum helpers that operate on passed-in qubits; qubit ownership is by reference.

## Error rules and constraints
- Single inheritance only; multiple inheritance is disallowed.
- Static methods cannot be virtual or override.
- Override must match parameter and return types exactly.
- Constructors must return the class type; destructors must return `void` and take no parameters.
- Visibility conflicts (e.g., multiple visibility modifiers) are errors.
- Static classes may only contain static members; constructors/destructors/instance fields or methods in a static class are errors.

## Usage checklist
- Prefer `private`/`protected` for fields; expose behaviour via methods.
- Mark methods `virtual` only when they are intended to be overridden; otherwise keep them non-virtual.
- Use `final` on variables to enforce immutability where appropriate.
- Keep quantum-side effects contained in `@quantum` methods; use classical methods for orchestration and state.
