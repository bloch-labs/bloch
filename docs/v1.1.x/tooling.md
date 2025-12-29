---
title: Tooling
---

Operate Bloch 1.1.x with the CLI, module loader, and shot orchestration.

## CLI essentials
```bash
bloch app.bloch                    # runs app.bloch (and its imports)
bloch app.bloch --emit-qasm        # write app.qasm
bloch app.bloch --shots=512        # legacy shot flag; overridden by @shots on main
bloch app.bloch --echo=all         # print echo output on every shot
bloch app.bloch --echo=none        # suppress echo
```
`@shots(N)` on `main` is the authoritative shot count. When both are present, the annotation wins and the CLI warns if values differ.

## Modules
- Resolve imports relative to the current file, then configured search paths, then CWD.
- All imported modules merge into a single program; exactly one `main` is allowed.
- Import cycles are detected and rejected with a semantic error.

## Working with examples
```bash
bloch examples/05_teleport_class.bloch --shots=1024
bloch examples/06_maxcut_c4_class.bloch --emit-qasm
```

## QASM artifacts
- `<source>.qasm` is written next to the entry file after execution.
- Use QASM logs to validate circuit shape and to interop with external simulators.

## Debugging tips
- Use `echo` for classical state and `@tracked` for quantum histograms.
- If you see access errors, check member visibility and static vs. instance context.
- Constructors must match their class name; `= default` constructors cannot bind qubit fields.

## Environment checks
```bash
bloch --version
bloch --help
```
