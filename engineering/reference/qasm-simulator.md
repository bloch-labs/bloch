# QASM Simulator
Source: `src/bloch/runtime/qasm_simulator.hpp`, `src/bloch/runtime/qasm_simulator.cpp`

Inputs:
- Gate invocations from the runtime (`h`, `x`, `y`, `z`, `rx`, `ry`, `rz`, `cx`)
- Qubit indices allocated by the runtime
- Reset and measure requests

Outputs:
- Updated statevector
- OpenQASM log (header + qreg/creg + gate statements) when logging is enabled
- Measurement results (ints) returned to runtime
- `BlochError` with `Runtime` category on invalid operations

Behaviour:
The simulator keeps an ideal statevector, doubling its size for each allocated qubit and zeroing the |1> subspace. Gates are applied with blocked matrix multiplication and minimal branching. Measurement samples the appropriate probability, collapses the state, and marks qubits so later gate calls on measured qubits fail fast. Reset renormalises the |0> subspace, clears measurement marks, and allows index reuse. QASM logging is optional per evaluator instance; even when logging is off, register declarations still reflect the allocated qubits.

Invariants/Guarantees:
- Throws on invalid qubit indices or gate calls on measured qubits.
- `stateSize()` is always a power of two, matching the number of allocated qubits.

Edge Cases/Errors:
- Division by zero is not present in gate math; main risks are invalid indices and post-measurement operations.
- Logging suppression only affects gate lines; register declarations still reflect allocated qubits.

Extension points:
- Add new gates by implementing matrix application and QASM emission; ensure runtime dispatch matches.

Tests:
- `tests/test_runtime.cpp`

Related:
- `runtime-evaluator.md`, `semantics.md`
