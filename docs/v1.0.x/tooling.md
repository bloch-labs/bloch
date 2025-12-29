---
title: Tooling
---

Operate Bloch 1.0.x with the CLI and a handful of guardrails for reproducible runs.

## CLI essentials
```bash
bloch program.bloch              # single-shot execution
bloch program.bloch --shots=512  # multi-shot; aggregates @tracked counts
bloch program.bloch --emit-qasm  # write program.qasm next to the source
bloch program.bloch --echo=all   # print echo output on every shot
bloch program.bloch --echo=none  # suppress echo entirely
```
`--shots` is the only way to request multiple runs in 1.0.x. Use `@tracked` on qubits to populate the aggregated table. Echo output defaults to the last shot when multiple shots are requested.

## Working with examples
```bash
bloch examples/02_bell_state.bloch --shots=1024
bloch examples/03_deutsch_jozsa.bloch --emit-qasm
```
The `examples/README.md` describes each sample and expected outcomes.

## QASM artifacts
- Outputs are written as `<source>.qasm` in the same directory as the `.bloch` file.
- The OpenQASM log includes every gate and measurement in program order.

## Debugging tips
- Use `echo` to print intermediate classical values (array contents included).
- Add `@tracked` to qubit registers you need histograms for; leave scratch qubits untracked to reduce noise.
- If a variable must never change, mark it `final` to catch accidental mutation.

## Environment checks
```bash
bloch --version   # verify install
bloch --help      # view flags and behavior notes
```
