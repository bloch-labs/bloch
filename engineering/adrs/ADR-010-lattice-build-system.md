# ADR-010: Lattice Build System and Ecosystem Separation

## Owner
Akshay Pal

## Status
Proposed

## Delivered In
Planned for v2.0.0

## Context
- Bloch v1.1.x–v1.2.x focuses on:
  - Language features.
  - A language-level import/package system (ADR-009).
  - A versioned stdlib distributed alongside the language (from v1.2.0).
- Build orchestration in 1.x is intentionally simple:
  - Users invoke `bloch` directly.
  - Users can `import` custom classes into `main.bloch`
- As Bloch moves to **bytecode + JIT** in v2.0.0, there is a clear need for:
  - A proper **project model** (modules, targets).
  - **Dependency management** (versions, locks, registries).
  - **Orchestration** of compiler, runtime, and tooling in a single, cohesive UX.
- We want to keep the core language toolchain focused:
  - `blochc`: compiler only.
  - `bloch`: runtime/JIT/VM only.
  - A **separate build system** will handle orchestration, similar to how `mvn` is separate from `javac/java`, `cargo` is separate to `rustc` and `pip` is separate to `python`.

## Decision
- **Introduce Lattice as the official Bloch build system**
  - Name: **Lattice** (e.g. `lattice build`, `lattice test`).
  - Packaged and versioned separately from the Bloch language repo:
    - Lattice lives in its own GitHub repository (https://github.com/bloch-labs/lattice).
    - Lattice is released and versioned independently of Bloch.
  - Lattice is the **official** build system for Bloch:
    - Official docs and examples assume Lattice for non-trivial projects.
    - The ecosystem is free (and encouraged) to build alternative tools on top of the same compiler/runtime contracts.

- **Clear separation of responsibilities**
  - `blochc` (compiler):
    - Parses, typechecks, and compiles Bloch source to IR/bytecode.
    - Knows nothing about dependency resolution, remote repositories, or project graphs.
    - Exposes a stable CLI and/or library API for Lattice to orchestrate.
  - `bloch` (runtime/JIT):
    - Executes bytecode, manages runtime libraries, and provides a stable application binary interface (ABI) for compiled artifacts.
    - Does not manage dependency resolution or project structure.
  - `lattice` (build system):
    - Defines the **project model** (modules, targets, test suites).
    - Manages **dependencies**:
      - The Bloch stdlib (from v1.2.0).
      - Future community packages and registries.
    - Orchestrates:
      - Compilation (`blochc` invocations).
      - Running, testing, packaging (calls into `bloch` and other tools).

- **Implementation language**
  - Implementation language is left open in this ADR but should prioritise:
    - Strong cross-platform story.
    - Single static binary or minimal runtime dependency.
    - Great tooling ecosystems for CLI and concurrency.
  - Rust and Go are both strong candidates; a later design (DES) can choose concretely.

- **Timeline and scope**
  - v1.1.0:
    - Language-level `package` and `import` semantics (ADR-009, DES-007).
    - No dedicated build system.
  - v1.2.0:
    - Stdlib shipped with the language, installed to user-local data directories.
    - Compiler knows how to locate the stdlib (via well-defined defaults + environment override).
    - Still no Lattice; dependency graph is trivial (stdlib + project).
  - v2.0.0:
    - Bytecode + JIT architecture in core Bloch.
    - **Initial release of Lattice (v1.0.0)** in a separate repository, targeting:
      - Single-project builds.
      - Integration with stdlib and basic dependency metadata.
      - Room to grow into a full-blown package manager over time.

## Alternatives Considered
- **Alternative 1: No separate build system (compiler does everything)**
  - Pros:
    - Fewer binaries; simpler mental model for new users.
    - Centralised implementation in a single codebase.
  - Cons:
    - Tight coupling of language evolution with build/dependency concerns.
    - Harder for the community to build alternative tools or experiment with different dependency models.
    - Risk of `blochc` becoming a monolith that mixes compilation, packaging, testing, and deployment concerns.
  - Outcome:
    - **Rejected**; we explicitly separate compiler/runtime and build system responsibilities.

- **Alternative 2: Delay build system beyond v2.0.0**
  - Pros:
    - More time to observe user needs and JIT behaviour.
    - Avoid taking on build/dependency complexity too early.
  - Cons:
    - Users would lack a first-class, idiomatic way to build and test Bloch projects using the new bytecode model.
    - Third-party tools might fill the gap in fragmented ways, creating migration pain later.
  - Outcome:
    - **Rejected**; v2.0.0 is the right inflection point to deliver an official, cohesive story.

## Consequences
- Bloch’s core toolchain remains **focused**:
  - Compiler and runtime are small, well-defined components with stable CLIs.
- The community gets:
  - A clear, official build tool (`lattice`) for day-to-day development once v2.0.0 lands.
  - The freedom to write alternative build systems without being blocked by monolithic compiler design.
- Documentation and onboarding flows:
  - 1.x docs emphasise direct `bloch` usage.
  - 2.x docs emphasise `lattice` for project-level builds, while still documenting direct compiler/runtime usage where needed.

