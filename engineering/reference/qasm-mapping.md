---
title: QASM Mapping
---
# QASM Mapping

Bloch emits OpenQASM 2.0 for quantum operations performed by the interpreter and simulator. The CLI always writes `<file>.qasm` next to your source; use `--emit-qasm` to also print it to stdout.

## Gate mappings

Single-qubit gates:

| Bloch | OpenQASM |
| --- | --- |
| `h(q)` | `h q[i];` |
| `x(q)` | `x q[i];` |
| `y(q)` | `y q[i];` |
| `z(q)` | `z q[i];` |
| `rx(q, theta)` | `rx(theta) q[i];` |
| `ry(q, theta)` | `ry(theta) q[i];` |
| `rz(q, theta)` | `rz(theta) q[i];` |

Two-qubit gate:

| Bloch | OpenQASM |
| --- | --- |
| `cx(ctrl, tgt)` | `cx q[c], q[t];` |

Notes
- `q[i]` refers to the underlying simulator qubit at index `i`. When using single `qubit` variables, the simulator assigns an index for that allocation; when using `qubit[]`, `q[k]` maps directly to an index.
- Rotation angles are emitted as-is (float values in radians).

## Measurement and classical bits

- Statement form: `measure q;` becomes `measure q[i] -> c[i];` in the log and collapses the qubit.
- Expression form: `bit b = measure q;` also produces a measurement result and returns a `bit` value to the program.

The simulator maintains a classical register `c[...]` for recorded outcomes in the QASM log.

## Example

Bloch:

```
@quantum
function bell(qubit a, qubit b) -> void {
  h(a); cx(a, b);
}

function main() -> void {
  qubit q0; qubit q1;
  bell(q0, q1);
  measure q0; measure q1;
}
```

Emitted QASM:

```
OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
creg c[2];
h q[0];
cx q[0],q[1];
measure q[0] -> c[0];
measure q[1] -> c[1];
```

See also
- Built-ins and signatures: [Built-ins](./builtins.md)
- Runtime and simulator details: [Runtime Evaluator](./runtime-evaluator.md) and [QASM Simulator](./qasm-simulator.md)
