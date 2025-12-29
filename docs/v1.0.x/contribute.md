---
title: Contribute
---

Bloch 1.0.x contributions focus on stability and clarity. Use this checklist to submit changes that respect the 1.0.x surface (no classes/imports/`@shots`).

## Set up a dev environment
```bash
git checkout v1.0.2
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Development guidelines
- Keep programs functional and single-file; the 1.0.x grammar has no classes or imports.
- Use `final` for constants and loop bounds to avoid hidden mutation.
- Gate additions must include semantic checks and QASM emission.
- Update or add runnable examples in `examples/` when you change user-visible behavior.
- Favor small `@quantum` helpers for quantum-only logic; keep `main` classical.

## Testing changes
```bash
ctest --test-dir build --output-on-failure
./build/bin/bloch examples/02_bell_state.bloch --shots=256 --emit-qasm
```
Validate that tracked histograms and QASM output still match expectations.

## Documentation updates
- Edit only the `docs/v1.0.x/` tree for 1.0.x behavior; avoid forward references to later releases.
- Keep examples in code blocks and mirror the tone of the rest of the guide.
