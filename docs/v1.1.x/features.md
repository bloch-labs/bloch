---
title: Features
---

Bloch 1.1.x adds structural and execution features while preserving the functional quantum core.

## Language surface
- **Classes**: Encapsulate state with visibility (`public`/`private`/`protected`), static and instance members, inheritance (`extends`), virtual/override dispatch, constructors/destructors (including `= default`), and explicit `destroy`.
- **Modules**: `import path.to.module;` merges other `.bloch` files; exactly one `main` may exist across the import graph.
- **Annotations**: `@quantum` for quantum helpers, `@tracked` on qubits/fields, and `@shots(N)` on `main` to request repeated execution.

## Execution model
- **Statevector backend** with emitted OpenQASM logs. Quantum operations compose into a unitary \(U\); measurement projects and records classical bits.
- **Shot orchestration**: `@shots(N)` on `main` drives N executions; CLI `--shots` is preserved for compatibility but defers to the annotation when both are present.
- **Tracked observability**: Tracked qubits and tracked qubit fields accumulate histograms across shots; warnings appear if nothing was tracked.

## Object model
- **Constructors/destructors**: Explicit `constructor` and `destructor` blocks. `= default` constructors map parameters to fields (excluding qubit fields) and run automatically on `new`.
- **Lifetime hooks**: `destroy obj;` invokes the destructor and marks the object; the runtime also runs destructors during GC when objects fall out of scope.
- **Static classes**: `static class C { ... }` holds only static members and no constructors/destructors.

## Quantum surface
- **Gate set**: Built-ins remain `h`, `x`, `y`, `z`, `rx`, `ry`, `rz`, `cx`.
- **Measurement/reset**: Same semantics as 1.0.x; measurement returns `bit`, `reset` clears a qubit’s measured flag.
- **Class-aware quantum methods**: Methods may be annotated `@quantum` and must return `void`, `bit`, or `bit[]`.

The simulator evolves states as
$$
|\psi_{\text{out}}\rangle = U_{n}\cdots U_{1}|\psi_{\text{in}}\rangle,\qquad\rho_{\text{shot}} = \sum_k p_k |k\rangle\!\langle k|\ \text{after measurement}.
$$

## QASM and diagnostics
- `--emit-qasm` writes `<file>.qasm` alongside the source.
- `echo` prints classical data; suppressed on intermediate shots unless `--echo=all` is set.
- Import resolution errors and annotation misuse surface as semantic errors before execution.

## Best practices
- Keep quantum-heavy methods annotated `@quantum` and return only `void`/`bit`/`bit[]`.
- Prefer `@shots` over CLI `--shots` to make shot counts part of source control.
- In classes, restrict qubit fields to be explicitly managed and tracked as needed; avoid default constructors that bind qubit fields.
- Use `destroy` when you need deterministic cleanup of class-managed resources; otherwise rely on automatic lifetime management.
