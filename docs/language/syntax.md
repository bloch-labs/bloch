---
title: Syntax & Operators
---
# Syntax & Operators

This page summarizes tokens, literals, operators, precedence, and common statement forms as implemented by the v1.0.0 RC parser.

## Tokens

- Identifiers: `[A-Za-z_][A-Za-z0-9_]*`
- Keywords: `function, return, if, else, for, while, measure, reset, final, void`
- Types: `int, float, char, string, bit, qubit`
- Built-ins: `echo` and quantum gate names (`h, x, y, z, rx, ry, rz, cx`)
- Annotations: `@quantum`, `@tracked`
- Comments: `//` to end of line

## Literals

- Integer: `123`
- Float: `3.14f` (must include `f` suffix)
- Bit: `0b` or `1b`
- Char: `'a'` (no escapes in v1.0.0)
- String: `"hello"` (may span lines; no escapes in v1.0.0)
- Array: `{1, 2, 3}`

## Operators

- Arithmetic: `+ - * / %`
- Comparison: `< <= > >= == !=` → bit
- Logical: `! && ||` → bit
- Bitwise: `~ & | ^` on `bit` and `bit[]`
- Assignment: `=` (right-associative in expressions)
- Postfix: `++ --` (only on `int` variables)
- Indexing and call chaining: `a[i]`, `f(x,y)` are composable: `f(a[i])`

<!-- Removed duplicate precedence diagram (kept in Grammar page) -->

## Statements

Bloch statements control flow and produce side effects. This section lists each statement with syntax and key rules.

### Block

```
{ statement* }
```

Creates a new lexical scope for names and `@tracked` collection.

### Variable declaration

```
[final] [@tracked]? type name (= expr)? ;
// Only qubits may be multi-declared
qubit q0, q1; // ok
int a, b;     // error
```

- `final` variables cannot be reassigned.
- `@tracked` is allowed on `qubit` or `qubit[]` only.

### Assignment

```
identifier = expr ;
```

Type-checked assignment. The right side may not be a `void` result.

### Expression statement

```
expr ;
```

Evaluates an expression for side effects, e.g., a function/gate call.

### Return

```
return ;          // in void functions only
return expr ;     // in non-void functions
```

All non-void functions must return along all paths.

### If / else

```
if (expr) { ... } [ else { ... } ]
```

The condition is any expression yielding a `bit` (0b/1b is falsy/truthy).

### While

```
while (expr) { ... }
```

Loops while the `bit` condition is truthy.

### For

```
for ( init? ; condition ; update ) { ... }
// init: declaration or expression statement
```

Typical C-style loop. `condition` is evaluated each iteration; `update` runs after the body.

### echo

```
echo(expr) ;
```

Prints a human-friendly representation. Concatenate with `+`.

### reset

```
reset q ;
```

Resets a `qubit` (or `qubit[]` element) to `|0>`.

### measure (statement form)

```
measure q ;
```

Collapses a qubit and records an outcome for `@tracked`. For an immediate `bit` value, use the expression form: `bit b = measure q;`.

### Conditional statement (`? :`)

```
expr ? statement : statement
```

Statement-level conditional; unlike many languages, this is not an expression in Bloch. Both branches must be statements.

