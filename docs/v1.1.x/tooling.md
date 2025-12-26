---
title: Tooling & CLI
---

Command-line switches, build tasks, and debugging patterns for 1.1.x.

## CLI usage
```bash
bloch [options] <file.bloch>
```
Common options:
- `--shots=N` — repeat execution N times and aggregate `@tracked` outcomes.
- `--emit-qasm` — print generated OpenQASM (also writes `<file>.qasm` next to your source).
- `--echo=all|none` — override automatic echo suppression during multi-shot runs.
- `--update` — self-update to the latest release.
- `--version`, `--help`.

## Run and inspect examples
```bash
bloch examples/02_bell_state.bloch --shots=512
bloch examples/04_grover_search.bloch --emit-qasm
```

## Outputs and files
- `<file>.qasm` is written alongside your source after each run.
- `--emit-qasm` also prints the QASM log to stdout.
- Tracked summaries show counts and probabilities when `--shots` > 1; warnings about unmeasured qubits appear before echoes.

## Debugging tips
- Keep `@tracked` on qubits you want to observe; it only affects reporting, not state.
- For per-shot debugging plus aggregates, use `--shots=N --echo=all`.
- Reset before reusing qubits: `reset q;` clears the measured marker.

## Build/test shortcuts
- Configure & build release:  
  `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel`
- Run tests:  
  `ctest --test-dir build --output-on-failure`
- For debug symbols: configure with `-DCMAKE_BUILD_TYPE=Debug`.
