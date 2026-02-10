# Bloch Class System (Introduced in v1.1.x)

Bloch includes a full OO model with single inheritance, virtual dispatch, and generic classes. This guide is user-facing: how to declare, inherit, construct, and use classes safely.

## Declaring Classes
```bloch
class Point {
    public int x;
    public int y;
    public constructor(int x, int y) -> Point { this.x = x; this.y = y; return this; }
}
```
- Modifiers: `abstract` (not instantiable), `static` (type-only container).
- Visibility on members: `public | private | protected`.

## Inheritance & Dispatch
```bloch
class Base { public virtual function get() -> int { return 0; } }
class Derived extends Base {
    public override function get() -> int { return 1; }
}
```
- Single inheritance only (`extends Base`).
- `virtual` enables override; `override` required to replace a virtual base method.
- `abstract` classes or classes with bodyless virtual methods cannot be instantiated.

## Constructors & Destructors
- Syntax: `constructor(params) -> ClassName { ... }`
- `super(...)` only as the first statement.
- `= default` form allowed; static classes cannot have ctors/dtors.
- One destructor max; `destructor() -> ClassName` (optional `= default`).
- `final` fields are single-assignment: declaration initialiser or constructor assignment, not both.

## Fields
- Modifiers: `static`, `final`, `tracked`.
- `final static` fields must be initialised at declaration.
- `final` instance fields may be assigned in constructors only when they do not already have a declaration initialiser.
- Every constructor in the declaring class must initialise each uninitialised `final` instance field exactly once, via a top-level constructor assignment.
- Constructors cannot assign inherited `final` fields.
- Arrays allowed; qubit arrays cannot be initialised with literals.
- Access via `this.field` or `Type.field` (if static).

## Generics
```bloch
class Box<T> { public T v; public constructor(T v) -> Box<T> { this.v = v; return this; } }
class LabeledBox<T> extends Box<T> { public string label; public constructor(string l, T v) -> LabeledBox<T> { super(v); this.label = l; return this; } }
Box<int> b = new Box<int>(1);
```
- Class type parameters with optional bounds: `class Foo<T extends Bar>`.
- Type arguments on use: `Foo<Baz>`; `extends` accepts type args.
- Runtime monomorphisation: first use of each type-argument set creates and caches a concrete specialisation.

## Method Overloading
```bloch
class Adder {
    public function add(int a, int b) -> int { return a + b; }
    public function add(float a, float b) -> float { return a + b; }
    public function add(Box<int> bx) -> int { return bx.v + 1; }
}
```
- Overloads share a name but must differ in parameter types/arity. Exact duplicates are rejected at compile time.
- Resolution is static & unambiguous: the call’s argument types must match exactly after generic substitution; otherwise you’ll get “method not found” or “ambiguous call”.
- Overrides are per-overload: to override, match the same parameter list as the virtual in the base.

## Nullability
- Only class references (including generics) may be `null`. Primitives and arrays are non-nullable.
- Null only in `==` / `!=` comparisons; member access on null raises `"null reference"`.
- `destroy null` is a no-op.

## Static Context Rules
- Instance members require an object; static members accessed via the type.
- `this` and `super` are illegal in static methods/contexts.

## Functions & Annotations
- Top-level functions: `function name(params) -> type { ... }`.
- `@quantum` functions must return `bit`, `bit[]`, or `void`.
- `@shots(N)` allowed on `main` only.

## Quick Reference
- Non-nullable: `int`, `float`, `bit`, `char`, `string`, `qubit`, primitive/complex arrays.
- Nullable: class refs (incl. generic instantiations).
- Forbidden: static class with instance fields/ctors/dtors; overriding static methods; assigning `null` to primitives/arrays.
