---
title: Getting Started
---
# Getting Started

This quick guide shows how to write and run your first Bloch program, emit OpenQASM, and iterate with multi-shot execution.

## Install

Download a release and install per README instructions, or build from source. Verify:

```
bloch --version
```

## Hello, Bloch

Create `hello.bloch`:

```
function main() -> void {
  echo("Hello, Bloch!");
}
```

Run it:

```
bloch hello.bloch
```

Output:

```
Hello, Bloch!
```

## First quantum program

```
@quantum
function flip() -> bit {
  qubit q;
  h(q);
  return measure q;
}

function main() -> void {
  bit b = flip();
  echo("Measured: " + b);
}
```

Run and emit QASM:

```
bloch --emit-qasm flip.bloch
```

The interpreter writes `<file>.qasm` alongside your source and prints to stdout when `--emit-qasm` is set.

## Multi-shot runs and @tracked

Annotate qubits or qubit registers with `@tracked` to collect outcome counts across runs:

```
function main() -> void {
  @tracked qubit q;
  h(q);
  measure q;
}
```

Run 100 shots and show an aggregate table:

```
bloch --shots=100 tracked.bloch

[INFO]: suppressing echo; to view them use --echo=all
Shots: 100
Backend: Bloch Ideal Simulator
Elapsed: 0.004s

qubit q
outcome | count |  prob
--------+-------+-----
0       |    53 | 0.530
1       |    47 | 0.470
```

Use `--echo=all` to print `echo()` output every shot (suppressed by default when shots > 1).
