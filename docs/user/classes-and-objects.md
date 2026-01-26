# Classes and Objects

Bloch v1.1.x introduces a class system letting you you package fields, methods, and constructors into custom Objects. 
A minimal example:
```bloch
class Counter {
    private int value;
    public constructor(int start) -> Counter { this.value = start; }
    public function inc() -> void { this.value = this.value + 1; }
    public function get() -> int { return this.value; }
}
```

Visibility can be `public`, `private`, or `protected` (subclasses can see protected members). A `static class` is just a namespace for static members—no instances or constructors. Normal classes may have static and instance members; you need `new` to create an instance.

Constructors use `constructor(params) -> ClassName { ... }` or `= default` to bind parameters to matching fields. Single inheritance is declared with `extends Base`, and you can call `super(...)` as the first statement to run a base constructor.

Destructors (`destructor() -> void { ... }`) run base-to-derived when objects are reclaimed; cycles with tracked fields may live until program exit.

Fields may be `final`, `static`, or `@tracked`. Array fields can have fixed sizes; `qubit[]` fields cannot be initialised directly. Methods may be `virtual` and overridden with `override`; static methods cannot be virtual or override. You can mark methods `@quantum` when they return `bit`, `bit[]`, or `void`.

Instantiate and destroy like this:
```bloch
Counter c = new Counter(0);
c.inc();
echo(c.get());
destroy c;
```
