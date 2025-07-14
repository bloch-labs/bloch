# Bloch Examples

This directory contains example programs written in Bloch showcasing basic and hybrid quantum algorithms. These samples are meant for reference while the language and compiler are under development.

- **superposition.bloch** – prepares a single qubit in superposition and
  measures the result.
- **bell_pair.bloch** – generates an entangled Bell state of two qubits
  and prints both measurement outcomes.
- **ghz.bloch** – creates a three qubit GHZ state and measures each
  qubit.
- **coin_flip.bloch** – demonstrates an `@quantum` function used inside
  a classical loop to count heads.
- **hybrid_control.bloch** – shows a measurement result controlling
  subsequent quantum gates.

Compilation to OpenQASM or execution is not yet available, but these files demonstrate the intended syntax of the language.