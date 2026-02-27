# Quantum Programming

Bloch treats quantum data explicitly. Declare qubits with `qubit q;` or `qubit[N] qreg;`, use
`reset q;` to return a qubit to `|0>`, and `measure q;` to collapse to a `bit`. Applying gates
after a measurement raises a runtime error unless you reset first, and `qubit[]` cannot be
initialised with literals.

Built-in gates cover common single- and two-qubit operations: `h`, `x`, `y`, `z`, `rx`, `ry`,
`rz`, and `cx`. See [Built-ins and Quantum Gates](./builtins-and-gates.md) for a full reference.

Mark quantum functions with `@quantum` and return `bit`, `bit[]`, or `void`:
```bloch
@quantum
function flip() -> bit {
    qubit q;
    h(q);
    return measure q;
}
```

To collect outcomes automatically, annotate qubit variables or fields with `@tracked` (see
`annotations-and-tracking.md`). Multi-shot runs come from `@shots(N)` on `main()` or the
`--shots=N` CLI flag (deprecated in v2.0.0), which aggregate tracked values across runs.

Each execution produces an OpenQASM file next to your source; add `--emit-qasm` to print it
as well.
