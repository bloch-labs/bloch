# Project Example: Quantum Risk Microservice (Bloch v2-style)

This folder sketches how a Bloch codebase can look when you start structuring it like a Java/C++ application instead of a single script. It uses the new `import` system to split code into modules and keeps static helpers in their own file.

```
project_example/
├── README.md                          # you are here
└── src/
    ├── main.bloch                     # entrypoint (requires a main() function)
    ├── QuantumRiskService.bloch       # application/service layer
    ├── circuits/
    │   └── FourQubitRiskOracle.bloch  # reusable quantum circuit class
    └── lib/
        └── MathUtils.bloch            # static helper functions/constants
```

## Running

From repo root:
```bash
bloch project_example/src/main.bloch --shots=256
```

`main` constructs a `QuantumRiskService`, which internally instantiates a `FourQubitRiskOracle` to run a toy cost + mixer layer on four qubits. Measurements are tracked to get a histogram of outcomes.

## Module boundaries
- `import QuantumRiskService;` pulls the service type into `main.bloch`.
- `QuantumRiskService` imports `circuits.FourQubitRiskOracle` and `lib.MathUtils` for orchestration and simple classical math.
- `MathUtils` is a `static class` with only static members (no constructor). Everything else is a normal class with explicit constructors.

## Why this structure?
- Mirrors a microservice layout: an entrypoint, a service layer, reusable circuit modules, and shared utilities.
- Keeps quantum kernels (@quantum methods) close to their owning classes, while classical orchestration sits in `main`/service classes.
- Demonstrates namespacing: `circuits.*` for quantum kernels, `lib.*` for helpers, `main.bloch` as the application root.

## Extending
- Add more circuits under `src/circuits/` and expose them through `QuantumRiskService`.
- Introduce configuration classes (e.g., `ConfigLoader`) and dependency-inject them into the service constructor.
- Wrap `main` with CLI arg parsing (once supported) to drive different betas/gammas or backends.
```
