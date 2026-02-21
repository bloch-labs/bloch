# Types and Data

Bloch has a compact set of primitives plus arrays and class references.

## Primitives
- `int`, `long`, `float`
- `bit` (measurement / bitwise value)
- `boolean` (logical truth value)
- `char`, `string`
- `qubit`
- `void` (functions only)

## Arrays
Dynamic arrays use `Type[]`, fixed-size arrays use `Type[N]` where `N` is a compile-time
constant `int` (literal or `final int`).

```bloch
int[] xs = {0, 1, 2};
long big = 123L;
long[] ids = {1L, 2L, 3L};
final int n = 3;
float[n] ys;
qubit[2] qreg;
```

Array element types can be primitives or `qubit`. Class arrays are not supported. The runtime
supports one-dimensional arrays only.

Arrays and primitives are non-nullable.

## Classes
Class types are named types such as `Point` or `Box<int>`. Only class references may be `null`:

```bloch
class Node { 
    public Node next; 
    public constructor() -> Node { 
        this.next = null; 
        return this;
        } 
    }
```

## Defaults
Uninitialised values default predictably:
- `int`/`long`/`float`/`bit`/`boolean` -> `0`/`0L`/`0.0f`/`0b`/`false`
- `char` -> `\0`
- `string` -> `""`
- `qubit` -> `|0>`
- arrays -> filled with the element default

Tracked `qubit` primitives (annotated with `@tracked`) record qubit outcomes; see `annotations-and-tracking.md`.
