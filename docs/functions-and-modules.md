# Functions and Modules

Functions are declared explicitly with typed parameters and a return type:
```bloch
function add(int a, int b) -> int { return a + b; }
```

`void` returns are allowed. Variables are block-scoped, and `final` makes them read-only after
initialisation. Top-level functions are not overloadable.

## Annotations
- `@quantum` marks functions that run quantum code and must return `bit`, `bit[]`, or `void`.
- `@shots(N)` applies to `main()` to run multiple shots without passing `--shots=N` (CLI flag to be deprecated in v2.0.0).

`@quantum` may not decorate `main()`.

## Packages and imports
Files may optionally declare a package once, before any imports or other declarations:
```bloch
package com.example.math;
```

Imports use dotted paths. You can import a symbol or an entire package directory:
```bloch
import com.example.QuantumMath;
import com.example.*;
```

Imports are resolved relative to the importing file first, then any configured search paths,
then the current working directory. Wildcard imports load all `.bloch` files in that directory
in sorted order.

All modules are merged into a single program. Exactly one `main()` must exist across all
imports.
