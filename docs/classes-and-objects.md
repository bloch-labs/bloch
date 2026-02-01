# Classes and Objects

Bloch includes a class system with single inheritance, visibility, virtual dispatch, and generics.

## Declaring classes
```bloch
class Counter {
    private int value;
    public constructor(int start) -> Counter { this.value = start; return this; }
    public function inc() -> void { this.value = this.value + 1; }
    public function get() -> int { return this.value; }
}
```

Visibility can be `public`, `private`, or `protected`. A `static class` is a namespace for
static members only (no instances or constructors).

## Constructors and destructors
Constructors use `constructor(params) -> ClassName { ... }` or `= default` to bind parameters
by name and type:
```bloch
class Point {
    public int x;
    public int y;
    public constructor(int x, int y) -> Point = default;
}
```

Destructors are optional and declared as `destructor() -> void { ... }` (at most one per class).

## Inheritance and overrides
```bloch
class Base { public virtual function get() -> int { return 0; } }
class Derived extends Base {
    public override function get() -> int { return 1; }
}
```

Overrides must match the base signature exactly. `super(...)` is only valid as the first
statement in a constructor, and only when a base class exists.

## Generics
```bloch
class Box<T> {
    public T value;
    public constructor(T value) -> Box<T> { this.value = value; return this; }
}

Box<int> b = new Box<int>(1);
```

Type parameters may be bounded: `class Pair<T extends Base> { ... }`.

## Null and destroy
Only class references may be `null`:
```bloch
class Node { public Node next; public constructor() -> Node { this.next = null; return this; } }
```

Use `destroy obj;` to explicitly release an object and run its destructor chain.

## Tracked fields
`@tracked` may decorate fields. Tracked qubit fields are recorded when the object is destroyed
(explicitly or at program shutdown).
