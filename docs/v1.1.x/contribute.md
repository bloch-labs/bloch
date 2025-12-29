---
title: Contribute
---

Guidance for contributing to Bloch 1.1.x. This line includes classes, imports, `@shots`, and object lifetimes.

## Set up
```bash
git checkout main
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Development guidelines
- Keep class APIs consistent: constructors must return the class type; default constructors cannot bind qubit fields; only one destructor per class.
- Respect visibility and static-context rules when adding methods or fields.
- Add module tests for any new `import`-sensitive behavior; imports must remain acyclic.
- `@quantum` methods and functions must return `void`, `bit`, or `bit[]`; `@shots` belongs only on `main`.
- Update examples under `examples/` to exercise new language surface (classes, imports, `@shots`).

## Testing
```bash
ctest --test-dir build --output-on-failure
./build/bin/bloch examples/05_teleport_class.bloch --shots=512 --emit-qasm
```
Check that tracked histograms, destructors, and imports behave as expected.

## Documentation
- Update `docs/v1.1.x/` in tandem with language changes. Keep the structure aligned with `docs/v1.0.x/`.
- Include code in fenced blocks, and document API contracts clearly.
