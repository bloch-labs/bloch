---
title: Language Guide
---
# Language Guide

This guide tours the Bloch language: types, variables, functions, control flow, arrays, and I/O. For the formal grammar see the [Grammar](../grammar) and for compile-time rules see [Semantics](./semantics).

## Essentials

- File extension: `.bloch`
- Comments: `//` to end of line
- Entry-point: `function main() -> void { ... }`
- Types: `void, int, long, float, char, string, bit, boolean, qubit` and arrays `T[]` or `T[N]`
- Literals: `123`, `3.14f`, `42L`, `1b`, `'c'`, `"text"`, `{1,2,3}`, `true`, `false`

## Packages and imports

Bloch supports optional Java-style packages without forcing every file into a package.
Package and import declarations must appear before any other declarations.

```
package com.example;
import com.example.QuantumMath;
import com.example.physics.*;

function main() -> void { ... }
```

Rules:
- `package` is optional. Files without a package are in the default package.
- `import foo.bar.Baz;` loads `foo/bar/Baz.bloch` and requires that file to declare
  `package foo.bar;`.
- `import foo.bar.*;` loads all `.bloch` files in `foo/bar/`, and each must declare
  `package foo.bar;`.
- `import Baz;` targets `Baz.bloch` in the default package.
- When running `bloch`, imports are resolved relative to the importing file's directory,
  then the current working directory.

Current limitation: names are still global, so two classes with the same name in different
packages are treated as duplicates.

## Variables

```
int a = 1;
final float pi = 3.1415f; // must be initialised and cannot be reassigned
long big = 9000000000L;   // 64-bit signed integer
string msg = "hello";
bit b = 1b;               // measurement bit (0b/1b)
boolean flag = true;      // classical boolean
qubit q;                  // allocates a new simulated qubit
int[] xs = {1, 2, 3};     // dynamic-sized array (size inferred from literal)
float[3] fs;              // fixed-size array, zero-initialised
```

Multi-declare only applies to qubits:

```
qubit q0, q1; // ok
int a, b;     // error
```

Immutability rules:
- `final` locals/top-level variables must be initialised at declaration.
- `final static` fields must be initialised at declaration.
- `final` instance fields may be initialised at declaration, or assigned exactly once in each constructor as a top-level statement.

## Functions

```
function add(int a, int b) -> int {
  return a + b;
}

@quantum
function sample() -> bit {
  qubit q; h(q);
  return measure q; // must return bit or void in @quantum
}
```

## Control flow

```
if (cond) { ... } else { ... }
while (i < 10) { ... }
for (int i = 0; i < 10; i = i + 1) { ... }

// Statement-form conditional
1 ? echo("yes"); : echo("no");
```

## Expressions and operators

Arithmetic: `+ - * / %` on `int`, `long`, and `float`. Mixed `int/long` promotes to `long`; any `float` promotes to `float`.
Comparison: `< <= > >= == !=` returns `boolean`.

Logical: `! && ||` (returns boolean; arrays unsupported for `!`).

Bitwise: `~ & | ^` on `bit` and `bit[]`. `~` flips bits, `& | ^` operate per element for arrays.

Casts and widening:
- `(long)expr` converts numeric values to 64-bit; `int` values can widen to `long` in assignments and calls.
- Numeric casts between `int`, `long`, `float`, `bit`, and `char` are permitted; narrowing may lose precision.

Postfix: `i++; i--;` only valid on `int` variables.

Function call and indexing:

```
rx(q, 1.0f);  // qubit rotation; see [Built-ins](../reference/builtins)
echo(xs[1]);  // indexing arrays
```

String concatenation: `"Answer: " + 42` → string.



## Arrays

Literals use `{ ... }`. Type must be an array of a primitive (no nested arrays for v1.0.0):

```
bit[] mask = {0b, 1b, 1b};
int[] a = {1, 2, 3};
long[] ids = {1L, 2L};
float[] f = {1.0f, 2.5f};
string[] s = {"a", "b"};
char[] cs = {'x', 'y'};

// Fixed-size with defaults
int[3] z; // {0,0,0}
```

Indexing is zero-based; index must be numeric. Constant negative indices like `a[-1]` are rejected at parse-time; dynamic negative indices error at runtime.

Per v1.0.0: `qubit[]` cannot be initialised from a literal; allocate with a size (`qubit[N] reg;`).

## I/O

`echo(expr);` prints human-friendly values (arrays as `{...}`), and concatenates via `+` with strings.

## Types at a glance
- Primitives: `void`, `int`, `long`, `float`, `char`, `string`, `bit`, `boolean`, `qubit`.
- Arrays: `T[]` (dynamic) or `T[N]` (fixed). No nested arrays.
- `bit` is a measurement result (0/1). `boolean` is the classical logic type used in conditionals and logical operators; comparisons return `boolean`.
- `qubit[]` cannot be initialised from a literal; allocate with a size.
