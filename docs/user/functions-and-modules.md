# Functions and Modules

Functions are declared explicitly with typed parameters and a return type:
```bloch
function add(int a, int b) -> int { return a + b; }
```
Returns may be `void`. Variables are block-scoped, and `final` makes them read-only after initialisation. Functions can be called before their definition inside the same program.

Annotations add intent: `@quantum` marks functions that run quantum code and must return `bit`, `bit[]`, or `void`. `@shots(N)` applies to `main()` to run multiple shots without passing `--shots=N`.

Imports use dotted paths relative to the caller or configured search paths:
```bloch
import math.LinearAlgebra;
function main() -> void { ... }
```
The module loader merges imports into one program and requires exactly one `main()`.

Keep functions small and pure where possible, return values instead of leaning on side effects, and group reusable code in modules you can import elsewhere.
