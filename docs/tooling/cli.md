---
title: CLI
---
# CLI

The `bloch` binary runs programs, emits OpenQASM, and supports multi-shot execution with `@tracked` statistics.

```
Usage: bloch [options] <file.bloch>

Options:
  --help          Show help and exit
  --version       Print version and exit
  --emit-qasm     Print emitted QASM to stdout
  --shots=N       Run the program N times and aggregate @tracked counts
  --echo=all|none Control echo statements (default: auto)

Behaviour:
  - Writes <file>.qasm alongside the input file.
  - When --shots is used, prints an aggregate table of tracked values.
```

Notes
- When `--shots > 1`, `echo()` output is suppressed unless `--echo=all` is set.
- The interpreter exits non-zero on lexical, parse, semantic, or runtime errors and prints a formatted error with line/column.
- Imports are resolved relative to the importing file's directory, then the current working directory.

See also: [Quantum Model](../language/quantum) for how measurement and reset behave.

### Example

```
bloch --shots=100 examples/entangled_tracked.bloch
```
