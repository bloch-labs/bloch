# Bloch Examples

This folder contains example programs written in **Bloch**, showcasing key language features and patterns for building quantum–classical workflows. Each example is designed to be runnable with the Bloch CLI and serves as both a tutorial and a testbed for the language.

- **superposition.bloch** – demonstrates integration of quantum and classical flows. Defines an `@quantum` function that prepares a qubit in superposition and measures it. The result is sampled in a loop and the totals are printed.
- **entangled_tracked.bloch** – tracks a `qubit[]` register prepared in a Bell state (H + CX) and measures both qubits to produce outcome counts like `00` and `11`.
