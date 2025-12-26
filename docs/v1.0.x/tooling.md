---
title: Tooling & CLI
---

Command-line switches and debugging tips for 1.0.x.

## CLI usage
```bash
bloch [options] <file.bloch>
```
Common options:
- `--shots=N` — repeat execution N times and aggregate `@tracked` outcomes.
- `--emit-qasm` — print generated OpenQASM (also writes `<file>.qasm`).
- `--echo=all|none` — override automatic echo suppression during multi-shot runs.
- `--update`, `--version`, `--help`.

## Run and inspect examples
```bash
bloch examples/02_bell_state.bloch --shots=512
bloch examples/03_deutsch_jozsa.bloch --emit-qasm
```

## Outputs and files
- `<file>.qasm` is written alongside your source after each run.
- `--emit-qasm` also prints the QASM log to stdout.
- Tracked summaries show counts and probabilities when `--shots` > 1; warnings about unmeasured qubits appear before echoes.

## Debugging tips
- Keep `@tracked` on qubits you want to observe; it only changes reporting.
- For per-shot debugging plus aggregates, use `--shots=N --echo=all`.
- Reset before reusing qubits: `reset q;`.
