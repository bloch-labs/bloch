---
title: Annotations
---

Annotations modify function or variable behavior.

### @quantum (functions/methods)
Marks a function or method as quantum-only.
```bloch
@quantum
function rotate(qubit q, float theta) -> void { ry(q, theta); }
```
Constraints: return type must be `void`, `bit`, or `bit[]`; `main` cannot be annotated.

### @tracked (variables/fields)
Accumulates measurement histograms across shots.
```bloch
class Probe {
    @tracked private qubit q;
    public constructor() -> Probe = default;
}

function main() -> void {
    @tracked qubit[2] q;
    // ...
}
```
Allowed on `qubit` variables and qubit fields/arrays.

### @shots(N) (main only)
Declares the number of shots to run `main`.
```bloch
@shots(2048)
function main() -> void { /* ... */ }
```
Rules:
- Only valid on the single `main` function across imported modules.
- If CLI `--shots` is also provided, the annotation wins; mismatches emit a warning.
