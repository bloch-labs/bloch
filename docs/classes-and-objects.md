# Classes and Objects

Bloch v1.1.x includes a class system with single inheritance, visibility, virtual dispatch,
and generics.

Every non-static class implicitly extends `Object`.

## Declaring classes
```bloch
class Counter {
  private int value;
  public constructor(int start) -> Counter { this.value = start; return this; }
  public function inc() -> void { this.value = this.value + 1; }
  public function get() -> int { return this.value; }
}
```

Visibility modifiers:
- `public` members are accessible from anywhere.
- `private` members are accessible only inside the declaring class.
- `protected` members are accessible inside the declaring class and its subclasses.

A `static class` is a namespace for static members only (no instances, constructors, destructors,
or instance members).

## Constructors and destructors
Constructors use `constructor(params) -> ClassName { ... }` or `= default` to bind parameters
by name and type:
```bloch
class Point {
  public int x;
  public int y;

  public constructor(int x, int y) -> Point = default;
  // Compiler resolves this to:
  // constructor(int x, int y) { this.x = x; this.y = y; return this; }
}
```

Rules:
- Every non-static class must declare at least one constructor.
- `super(...)` is only valid as the first statement in a constructor.
- If `super(...)` is omitted, an accessible zero-arg base constructor must exist.
- Destructors are optional and use `destructor() -> void { ... }` (at most one per class).

## Inheritance and overrides
```bloch
abstract class Base {
  public constructor() -> Base = default;
  public virtual function get() -> int;
}

class Derived extends Base {
  public constructor() -> Derived { super(); return this; }
  public override function get() -> int { return 1; }
}
```

Overrides must match the base signature exactly. Static methods cannot be `virtual` or
`override`.

## Method overloading
Methods in the same class may share a name if their parameter lists differ.

```bloch
class Adder {
  public constructor() -> Adder = default;
  public function add(int a, int b) -> int { return a + b; }
  public function add(float a, float b) -> float { return a + b; }
}
```

Duplicate signatures are rejected.

## Generics and diamond inference
```bloch
class Box<T> {
  public T value;

  public constructor(T value) -> Box<T> {
    this.value = value;
    return this;
  }
}

Box<int> a = new Box<int>(1);
Box<int> b = new Box<>();
```

Type parameters may be bounded: `class Pair<T extends Base> { ... }`.
`new Box<>()` is valid when the target type is known from context.

## Null and destroy
Only class references may be `null`:
```bloch
class Node {
  public Node next;
  public constructor() -> Node { this.next = null; return this; }
}
```

Use `destroy obj;` to explicitly release an object and run its destructor chain. `destroy null;`
is a valid no-op.

## Tracked fields
`@tracked` may decorate qubit fields. Tracked qubit fields are recorded when the object is destroyed
(explicitly or at program shutdown).
