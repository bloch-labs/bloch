# Bloch Language Specification

This document defines the Bloch language as implemented by the current release (see
`CHANGELOG.md`). It is the normative reference; the other guides in this folder are explanatory.

## Notation
- Grammar snippets use EBNF. Terminals appear in double quotes.
- `[ x ]` means optional, `{ x }` means repetition, and `x | y` means alternatives.
- When this spec says "must" or "may not", those requirements are enforced by the compiler or
  runtime.

## Source code
- Source files are UTF-8 text with the `.bloch` extension.
- Line and column numbers in diagnostics are 1-based.
- Line comments start with `//` and run to the end of the line.

## Lexical elements

### Identifiers
Identifiers match `[A-Za-z_][A-Za-z0-9_]*` and are case-sensitive.

### Keywords
The following tokens are reserved keywords and may not be used as identifiers:

`abstract`, `bit`, `boolean`, `char`, `class`, `constructor`, `default`, `destroy`, `else`,
`extends`, `false`, `final`, `float`, `for`, `function`, `if`, `import`, `int`, `long`,
`measure`, `new`, `null`, `override`, `package`, `private`, `protected`, `public`, `qubit`,
`reset`, `return`, `static`, `string`, `super`, `this`, `true`, `virtual`, `void`, `while`.

The built-in function `echo` and the quantum gate names (`h`, `x`, `y`, `z`, `rx`, `ry`, `rz`,
`cx`) are also reserved.

### Literals
- Integer: `123`
- Long: `123L`
- Float: `3.14f`
- Bit: `0b` or `1b`
- Boolean: `true` or `false`
- Char: `'a'` (exactly one character; no escapes)
- String: `"text"` (no escapes; may span lines)
- Null: `null`
- Array literal: `{expr, expr, ...}`

## Types

### Primitive types
`void`, `int`, `long`, `float`, `bit`, `boolean`, `char`, `string`, `qubit`.

### Array types
- Dynamic arrays: `T[]`
- Fixed-size arrays: `T[N]` where `N` is a compile-time constant `int` (a literal or `final int`).
- Arrays hold primitives (`int`, `long`, `float`, `bit`, `boolean`, `char`, `string`) or `qubit`.
  Class arrays are not supported, and only one-dimensional arrays are supported by the runtime.

### Class types and generics
- A class reference is a named type: `Point`, `Box<int>`.
- Only classes may be generic; functions and methods are not.
- Type arguments use `<>` and may include bounds on parameters: `class Box<T extends Base> { ... }`.
- Only class references may be `null`. Primitives and arrays are non-nullable.

## Declarations and scope
- Scope is lexical; inner scopes may not shadow existing names.
- Variables must be declared before use.
- `final` variables and fields may be assigned only once (fields are assignable in constructors).
- Only `qubit` variables may be multi-declared: `qubit q0, q1;`.

## Modules and imports
`package` and `import` statements appear at the top level. A file may declare at most one
package, and it must come before any imports or declarations:

```bloch
package com.example.math;
```

Imports use dotted names. You can import a symbol or a package wildcard:

```bloch
import com.example.QuantumMath;
import com.example.*;
```

Imports are resolved relative to the importing file first, then any configured search paths,
then the current working directory. Wildcard imports load all `.bloch` files in the directory,
and the loader validates that imported files declare the expected package (or no package for
the default package). Imports are merged into a single program. Exactly one `main()` function
must exist across all modules.

## Functions
Function declarations use explicit parameter and return types:

```bloch
function add(int a, int b) -> int { return a + b; }
```

- Parameters may not be `void`.
- Top-level functions are not overloadable; method overloading is supported in classes.
- `main()` is the program entry point and must be unique.

## Classes
Classes support single inheritance, visibility, static members, and generics.

```bloch
class Point {
  public int x;
  public int y;
  public constructor(int x, int y) -> Point { this.x = x; this.y = y; return this; }
}
```

Key rules:
- `static class` is a namespace for static members only (no instance fields, methods,
  constructors, or destructors).
- `abstract` classes may contain bodyless `virtual` methods.
- Constructors use `constructor(params) -> ClassName { ... }` or `= default` to bind parameters
  to fields by name and type.
- At most one destructor may be declared: `destructor() -> void { ... }`.
- Methods may be marked `virtual`; overrides must be marked `override` and match the base method
  signature exactly. Static methods cannot be `virtual` or `override`.
- `super(...)` is only valid as the first statement of a constructor, and only when a base class
  exists. `super` and `this` are not available in static contexts.
- `destroy expr;` requires a class reference or `null`.

## Expressions and operators
Bloch uses C-style precedence and left-to-right evaluation for most operators. Highlights:

- Arithmetic: `+ - * /` on numeric types (`int`, `long`, `float`). `%` is integer-only (`int` or
  `long`). `/` always returns `float`.
- Comparison: `< <= > >= == !=` return `boolean`.
- Logical: `! && ||` accept `boolean` or `bit` and return `boolean`.
- Bitwise: `~ & | ^` accept `bit` or `bit[]` (array operations are elementwise).
- String concatenation: `+` concatenates when either operand is a `string`.
- Postfix `++`/`--` apply only to non-`final int` or `long` variables.
- Indexing: `a[i]` for arrays. Index expressions accept numeric types (`int`, `long`, `bit`,
  `float`) and are coerced to an integer at runtime. Array element assignment requires an
  `int` or `long` index at compile time.
- Member access: `obj.field` or `obj.method(...)`.
- Allocation: `new Type(args)`.
- Casts: `(int|long|float|bit) expr` for explicit numeric/bit conversions. Casting to or from
  `boolean`, `char`, `string`, or class references is not supported.

`measure expr` is a prefix expression that returns a `bit` value.

## Statements
- Block: `{ ... }`
- Variable declaration: `[@tracked] [final] type name (= expr)? ;`
- Assignment: `identifier = expr ;`
- Return: `return;` or `return expr;`
- If/else: `if (cond) { ... } else { ... }` (cond must be `boolean` or `bit`)
- While: `while (cond) { ... }` (cond must be `boolean` or `bit`)
- For: `for (init?; cond; update) { ... }` (cond must be `boolean` or `bit`)
- Echo: `echo(expr);`
- Reset: `reset expr;` (qubit only)
- Measure (statement form): `measure expr;`
- Destroy: `destroy expr;`
- Conditional statement: `expr ? statement : statement` (statement-level only)

## Annotations
- `@quantum` marks functions or methods that contain quantum operations. They must return `bit`,
  `bit[]`, or `void`, and `@quantum` may not decorate `main()`.
- `@shots(N)` decorates `main()` to run the program `N` times and aggregate tracked results.
- `@tracked` decorates variables or fields. Local `@tracked` values are recorded at scope exit.
  Tracked fields are recorded when the object is destroyed (explicitly or at program shutdown).
  Only `qubit` and `qubit[]` values produce tracked output; other types are ignored.

## Built-ins and quantum operations
- `echo(expr)` prints a human-readable value.
- Quantum gates: `h`, `x`, `y`, `z`, `rx`, `ry`, `rz`, `cx` (all return `void`).
- `measure` and `reset` are available as statements; `measure` also has an expression form.

For a single reference with descriptions, see [Built-ins and Quantum Gates](./builtins-and-gates.md).

## Program execution
- Programs are lexed, parsed, and semantically checked before execution.
- The runtime is an interpreter with an ideal statevector simulator that emits OpenQASM 2.0.
- Each run writes `<file>.qasm` next to the input file; `--emit-qasm` prints it to stdout.
- `@shots(N)` or `--shots=N` repeats execution and aggregates tracked values. If both are present,
  the annotation wins and the CLI flag is ignored with a warning.

## Errors
Bloch reports lexical, parse, semantic, and runtime errors with 1-based line and column numbers.
Runtime errors include invalid casts, division by zero, illegal member access, and qubit misuse.
