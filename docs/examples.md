# Examples

The repository ships with runnable programs you can tweak:
- `examples/01_hadamard.bloch` — Apply H to a single qubit and measure.
- `examples/02_bell_state.bloch` — Prepare an entangled pair and observe correlated outcomes.
- `examples/03_deutsch_jozsa.bloch` — Oracle-based Deutsch–Jozsa sketch.
- `examples/04_grover_search.bloch` — Grover iteration outline.
- `examples/05_teleport_class.bloch` — Teleportation with classical scaffolding.
- `examples/06_maxcut_c4_class.bloch` — MaxCut on 4 nodes.
- `examples/07_null_demo.bloch` — Using `null` with class references.
- `examples/08_generics.bloch` — Generic classes and type bounds.
- `examples/multifile/*` — Imports across modules.

Run one with:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/bin/bloch examples/02_bell_state.bloch --emit-qasm
```

Use `--echo=all` for verbose output during multi-shot runs.
