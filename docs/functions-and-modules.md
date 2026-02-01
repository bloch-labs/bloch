# Functions and Modules

Functions are declared explicitly with typed parameters and a return type:
```bloch
function add(int a, int b) -> int { return a + b; }
```

`void` returns are allowed. Variables are block-scoped, and `final` makes them read-only after
initialisation. Top-level functions are not overloadable.

## Annotations
- `@quantum` marks functions that run quantum code and must return `bit`, `bit[]`, or `void`.
- `@shots(N)` applies to `main()` to run multiple shots without passing `--shots=N`.

`@quantum` may not decorate `main()`.

## Modules and imports
Imports use dotted paths and are resolved by the module loader:
```bloch
import math.LinearAlgebra;
```

All modules are merged into a single program. Exactly one `main()` must exist across all
imports.

Keep functions focused, return values instead of leaning on side effects, and group reusable
code in modules you can import elsewhere.
