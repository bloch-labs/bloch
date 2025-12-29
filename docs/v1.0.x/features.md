---
title: Features
---

Bloch 1.0.x is intentionally compact: a strongly typed, functional language that integrates classical control with quantum state evolution. Highlights below describe the execution model and the physics it exposes.

## Hybrid execution model
- **Statevector simulator**: Bloch emits OpenQASM and drives an ideal statevector backend. Programs evolve pure states unless you measure or reset.
- **Deterministic host control**: Classical branches and loops run on the host CPU while quantum operations queue in the simulator.
- **Entry point**: `function main() -> void` is the only entry. Functions can be annotated `@quantum` to mark pure-quantum helpers; `main` itself stays classical.

## Type system
- **Primitives**: `int`, `float` (literals require `f`), `bit`, `char`, `string`, and `qubit` handles. Arrays are fixed-size typed containers of any primitive.
- **Immutability**: `final` enforces single assignment for variables that must stay constant, which simplifies control/flow proofs.
- **No classes or imports**: All 1.0.x programs are functional modules in a single file.

## Quantum surface
- **Gates**: Built-ins cover single- and two-qubit operations `h`, `x`, `y`, `z`, `rx`, `ry`, `rz`, and `cx`.
- **Measurement and reset**: `measure q` yields a `bit`; `reset q` returns the qubit to \(|0\rangle\) and clears its measured flag.
- **Tracked qubits**: Annotate qubit variables with `@tracked` to accumulate outcome counts across shots.

Bloch models qubit evolution as:
$$
|\psi_{\text{out}}\rangle = U_{\text{program}} |\psi_{\text{in}}\rangle, \quad\text{with}\quad U_{\text{program}} = U_n \cdots U_2 U_1.
$$
Measurement projects \(|\psi\rangle\) into computational basis states and records a classical `bit`.

## QASM emission and observability
- Pass `--emit-qasm` to write `<file>.qasm` for inspection or execution elsewhere.
- `echo` prints classical values (including arrays) in run order; when `--shots` is used, echoes default to the final shot unless `--echo=all` is set.
- The runtime warns if tracked qubits were never measured, preventing silent data loss.

## Resource model
- **Simulator limits**: statevector space grows as \(2^n\); stay conservative beyond ~20 qubits on a laptop.
- **Deterministic qubit allocation**: `qubit` variables allocate simulator slots at declaration; `reset` reuses them.

## Best practices
- Keep quantum helpers small and mark them `@quantum` to make intent explicit.
- Track only the qubits you need histograms for; untracked qubits still simulate correctly but do not report aggregated outcomes.
- Prefer `final` on loop bounds and constants to avoid accidental mutation.
