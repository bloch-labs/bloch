---
title: Semantic Rules
---
# Semantic Rules

This page documents compile-time rules enforced by the semantic analyser (`src/bloch/semantics`). Errors report 1-based line and column.

## Names and scopes

- Lexical scoping for `{ ... }` blocks and functions.
- No shadowing: redeclaring a variable in any active scope is an error.
- Parameters live in the function scope and may not be redeclared.

## Packages and imports

- `package` is optional but must appear before any `import` or declarations.
- `import` statements load additional modules before semantic analysis.
- `import foo.bar.Baz;` resolves to `foo/bar/Baz.bloch` relative to the importing file's
  directory, then configured search paths, then the current working directory.
  The imported file must declare `package foo.bar;`.
- `import foo.bar.*;` loads all `.bloch` files in `foo/bar/`, and each must declare
  `package foo.bar;`.
- `import Baz;` targets `Baz.bloch` in the default package.
- `bloch.*` imports resolve from configured search paths first, so stdlib modules
  are preferred over project-local shadows.
- Names are still global, so duplicate class names across packages are treated as errors.

## Variables

- `final` variables must be initialised at declaration and cannot be assigned to after declaration.
- A variable must be declared before use.
- Only `qubit` may be declared with commas: `qubit q0, q1;`. Other types must be one per declaration.
- `@tracked` may only annotate `qubit` or `qubit[]`; otherwise it is an error.

## Class fields and constructors

- Non-static classes implicitly inherit from `Object` when `extends` is omitted.
- Every non-static class must declare at least one constructor.
- If a constructor omits `super(...)`, the base class must expose an accessible zero-arg constructor.
- Field initialisers are type-checked with the same null/type rules as variable initialisers.
- In static fields, the initialiser executes in static context (`this`/`super` are invalid).
- `final static` fields must be initialised at declaration.
- `final` instance fields with a declaration initialiser cannot be reassigned in constructors.
- `final` instance fields without a declaration initialiser must be assigned exactly once in every constructor of the declaring class, as a top-level constructor statement.
- Constructors cannot assign inherited `final` fields.
- Generic constructor diamond inference is supported when a target type is known
  (e.g. `Box<int> b = new Box<>();` and `b = new Box<>();`).

## Functions

- `@quantum` functions must return `bit`, `bit[]`, or `void`.
- Non-void functions must have at least one `return` along all paths.
- `void` functions may not `return` a value.
- Calling an undefined function is an error. Built-in gates are recognised as functions for arity/type checks.

## Calls and built-ins

- Built-in gate signatures (also used for type-checking):
  - `h(qubit) -> void`
  - `x(qubit) -> void`
  - `y(qubit) -> void`
  - `z(qubit) -> void`
  - `rx(qubit, float) -> void`
  - `ry(qubit, float) -> void`
  - `rz(qubit, float) -> void`
  - `cx(qubit, qubit) -> void`
- Argument counts and types are checked. Example: `rx(q, 1);` is invalid (`1` is `int` not `float`).
- Assigning the result of a `void` function (user-defined or built-in) is an error.

## Postfix operators

- `i++` and `i--` are only valid on non-`final` variables of type `int` only.
- Postfix targets must be variables (not expressions like `(a+b)++`).

## Arrays

- Element type must be a primitive (`int, float, char, string, bit, qubit`).
- `qubit[]` cannot be initialised with an array literal; use a fixed size (`qubit[N] r;`).
- Fixed-size arrays without an initialiser are default-initialised (`0`, `0.0`, `""`, `'\0'`, or newly allocated qubits).
- Array literals type-check all elements; permissive conversions are applied where implemented at runtime:
  - `int[]` accepts `int`, `bit`, and `float` (truncated to `int`).
  - `float[]` accepts `float`, `int`, and `bit` (promoted to `float`).
  - `bit[]` accepts only `bit`.
  - `string[]` accepts only `string`.
  - `char[]` accepts only `char`.
- Array element assignment checks index bounds and value type.
- Assignment into an array must target a variable array (e.g., `a[i] = ...;`).

## Measurement and reset

- `measure q;` is a statement that collapses a qubit and records a classical bit.
- `measure q` is also an expression that returns a `bit` value.
- `reset q;` returns the qubit to `|0>` even if previously measured.

## @tracked variables and shots

- `@tracked` qubits and registers accumulate outcome counts at scope exit. If not measured, an outcome of `"?"` is recorded.
- With `--shots=N`, the runtime executes the program `N` times and aggregates tracked outcomes across runs.
