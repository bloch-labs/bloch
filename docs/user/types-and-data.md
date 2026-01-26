# Types and Data

Bloch has a small set of primitives—`int`, `float`, `bit`, `string`, `char`, and `qubit`, plus arrays built on top. Unsized arrays use `Type[]`, and fixed-size arrays use `Type[N]` where `N` is a compile-time int. For example:
```bloch
int[] xs = {0, 1, 2};
int[3] ys;
qubit[2] qreg;
```
Qubit arrays cannot be initialised with literals; allocate them and then run gates.

Casting stays explicit: `(int)x`, `(float)x`, `(bit)x`. Division of numeric values promotes to `float`, while `%` remains integer-only. Casts to `char`, `void`, or class types are rejected.

Strings use double quotes; chars use single quotes and hold a single character. There are no escape sequences yet.

Tracked values work on any variable or field annotated with `@tracked`; see `annotations-and-tracking.md` for details.

Defaults are predictable: primitives zero out, strings start as `""`, chars as `'\0'`, arrays are filled with their type’s default, and qubits allocate in the |0> state.
