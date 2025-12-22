# Tutorials

> Curate a path from beginner to advanced. Each tutorial should be runnable with copy/paste commands and expected output.

- Intro: single-qubit gates and measurement.
- Intermediate: Bell pair, Deutsch–Jozsa, Grover search.
- Advanced: QAOA / VQE workflows.

Add tutorial pages (e.g., `bell-pair.md`, `grover.md`, `qaoa.md`) under this section and link to runnable examples in `examples/`. When adding them, include a local toctree:

````
```{toctree}
:maxdepth: 1
:caption: Tutorials
:glob:

tutorials/*
```
````
