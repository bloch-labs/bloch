---
title: Language Guide
---
# Language Guide

This guide tours the Bloch language: types, variables, functions, control flow, arrays, and I/O. For the formal grammar see the [Grammar](../grammar) and for compile-time rules see [Semantics](./semantics).

## Essentials

- File extension: `.bloch`
- Comments: `//` to end of line
- Entry-point: `function main() -> void { ... }`
- Types: `void, int, float, char, string, bit, boolean, qubit` and arrays `T[]` or `T[N]`
- Literals: `123`, `3.14f`, `1b`, `'c'`, `"text"`, `{1,2,3}`, `true`, `false`

## Variables

```
int a = 1;
final float pi = 3.1415f; // cannot be reassigned
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

Arithmetic: `+ - * / %` (int, float). Comparison: `< <= > >= == !=` (returns boolean).

Logical: `! && ||` (returns boolean; arrays unsupported for `!`).

Bitwise: `~ & | ^` on `bit` and `bit[]`. `~` flips bits, `& | ^` operate per element for arrays.

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
- Primitives: `void`, `int`, `float`, `char`, `string`, `bit`, `qubit`.
- Arrays: `T[]` (dynamic) or `T[N]` (fixed). No nested arrays.
- `bit` is used for booleans; comparisons return `bit`.
- `qubit[]` cannot be initialised from a literal; allocate with a size.
