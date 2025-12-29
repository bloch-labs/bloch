---
title: Classes
---

1.1.x introduces classes for encapsulation and reuse. Classes integrate with the type system and runtime evaluator.

## Declaration
```bloch
class Register {
    private @tracked qubit[2] q;
    public final int width = 2;

    public constructor() -> Register = default;

    @quantum
    public function bell() -> bit[2] {
        h(q[0]); cx(q[0], q[1]);
        return {measure q[0], measure q[1]};
    }
}
```

### Visibility and modifiers
- Members default to `private` in instance classes; `public` / `private` / `protected` are allowed.
- `static` members belong to the type. `static class C { ... }` forbids constructors, instance fields, and destructors.
- `abstract` marks a class that cannot be instantiated and may contain `virtual` methods without bodies.
- `virtual` enables overriding in derived classes; `override` requires a signature match with the base virtual method.

### Constructors
```bloch
public constructor(int shots) -> Experiment { this.shots = shots; }
public constructor() -> Experiment = default;
```
Rules:
- Constructors must return the class type.
- Non-static classes must declare at least one constructor.
- `= default` constructors map parameters to non-static, non-qubit fields of the same name and type; mismatches are semantic errors.
- `super(...)` may appear once, as the first statement in a constructor, to select a base constructor.

### Destructors and destroy
```bloch
destructor() -> void {
    // cleanup
}
...
destroy handle;  // runs destructor immediately
```
Rules:
- At most one destructor per class; return type must be `void` and it takes no parameters.
- `= default` is allowed for destructors.
- `destroy obj;` invokes the destructor and marks the object; the runtime also runs destructors for unreachable objects during collection.

### Methods
- Instance methods may use `this`; `super` is valid only in constructors.
- Methods can be `static`; instance members cannot be accessed from static context.
- Methods may be annotated `@quantum` and must then return `void`, `bit`, or `bit[]`.

### Inheritance
```bloch
abstract class Layer {
    virtual function apply(qubit q) -> void;
}

class HadamardLayer extends Layer {
    public override function apply(qubit q) -> void { h(q); }
}
```
- Single inheritance only (`extends Base`).
- Override checks enforce matching parameter and return types; static methods cannot be virtual/override.
- `protected` members are visible to subclasses; `private` are not.

### Tracking in classes
- Qubit fields may be annotated `@tracked`; their histograms are aggregated across shots alongside top-level tracked variables.
- Static qubit fields can also be tracked but remain shared across instances.

### Object creation
```bloch
MyClass inst = new MyClass(1, 2);
```
`new` selects a matching constructor; semantic checks ensure accessibility and signature alignment.
