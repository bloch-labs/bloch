# What is Bloch?

Bloch is an open-source, strongly typed **hybrid quantum programming language**. It lets you write classical and quantum code side by side in one language, with a single toolchain that stays **hardware-agnostic** so you can move between simulators and future quantum backends.

## Why Bloch?

- **Hybrid by design** — Use familiar control flow, types, and functions for classical logic; declare qubits and gates explicitly for quantum circuits. No need to stitch two languages together.
- **Strong typing** — Explicit types for parameters, returns, and variables reduce mistakes and make intent clear. The compiler catches many errors before you run, saving you expensive quantum compute time.
- **OpenQASM emission** — Every run produces OpenQASM 2.0 (and OpenQASM 3.0 soon), so you can inspect the generated circuit and integrate with tools that consume QASM.
- **Ideal simulator** — The built-in statevector simulator gives exact results for algorithm development and education; you can extend or replace the backend later.

## Typical use cases

- **Learning quantum computing** — Write small circuits (Hadamard, Bell states, simple algorithms) and see results and QASM in one place.
- **Algorithm prototyping** — Implement and debug quantum algorithms with classical scaffolding (e.g. teleportation, Grover, MaxCut) and multi-shot statistics.
- **Research and teaching** — Share readable, typed code and reproducible QASM output with collaborators or students.
- **Quantum application development** — Design and build real quantum applications.

## Next steps

- [Getting Started](./getting-started.md) — Install Bloch and run your first program (about 5 minutes).
- [Language Tour](./language-tour.md) — A quick walk through Bloch syntax.
- [Quantum Programming](./quantum-programming.md) — Qubits, gates, and `@quantum` functions.
