# DES-008: Lattice Build System – High-Level Design

## Scope
* ADR-010 (Lattice Build System)
* ADR-009 (Import and Package System)

## High Level Flow
- Lattice orchestrates Bloch compilation and execution without embedding compiler/runtime logic.
- Conceptual flows:
  - `lattice build`:
    - Read project descriptor → resolve dependencies → compute build graph → invoke `blochc` for each module → produce compiled artifacts (e.g. bytecode bundles).
  - `lattice run`:
    - Ensure build is up to date → call `bloch` with the correct entrypoint and artifact set.
  - `lattice test`:
    - Discover tests (per project config conventions) → build test targets → run via `bloch` with appropriate flags.

## Compilation (Compiler CLI Contract)
- Lattice treats the Bloch compiler as an external tool with a **stable CLI**:
  - Example shape (illustrative only; exact flags to be defined in a later compiler ADR/DES):
    - `blochc --source-root src --out build/main.bbc --stdlib-path <resolved-path> --target bytecode`
  - Required capabilities:
    - Multiple source roots (project code, generated code).
    - Configurable output directory / artifact name.
    - Overridable stdlib path (e.g. via `--stdlib-path`), consistent with ADR-009/DES-007.
    - Deterministic exits and diagnostics (machine-readable output is a plus).
- In v1.1.x–v1.2.x:
  - These CLI contracts are introduced and stabilised even before Lattice ships.
  - This allows users to write ad-hoc scripts and CI integrations that will later be smoothly migrated into Lattice.

## Semantic Analysis
- Lattice does **not** perform semantic analysis of Bloch code itself.
- It:
  - Coordinates **when** `blochc` is run and on which inputs.
  - Caches compilation results at the module/target level based on:
    - Source file hashes.
    - Compiler version and flags.
    - Dependency graph changes.
- All language semantics (import resolution, type checking, generics, etc.) remain within the Bloch compiler as described in ADR-001/009 and related DES docs.

## Runtime Architecture
- Lattice treats the Bloch runtime (`bloch`) as another external tool:
  - Example shape:
    - `bloch --run build/main.bbc --entry main.bloch.MainClass`
  - Lattice responsibilities:
    - Choosing which compiled artifacts to pass to `bloch`.
    - Managing runtime configuration (environment variables, flags) in a reproducible, declarative way.
    - Coordinating test execution and integration with external tools (coverage, profilers).
- The runtime itself:
  - Implements bytecode execution and JIT (v2.0.0+).
  - Exposes stable entrypoints for Lattice to call, but remains independent of project/dependency semantics.

## Dependency Management
- High-level goals (to be elaborated in future ADRs/DESs):
  - Model dependencies at the package/module level, aligned with the Bloch package/import system.
  - Support:
    - The Bloch stdlib (from v1.2.0).
    - Future community packages hosted in a Maven-like or registry-based layout.
  - Provide:
    - Lockfiles for reproducible builds.
    - Caching of downloaded/compiled artifacts.
- Lattice will:
  - Own the mapping from logical dependencies (e.g. `group:artifact:version`) to on-disk artifacts that `blochc`/`bloch` consume.
  - Use the same stdlib resolution conventions as the compiler:
    - User-local data directories (`~/.local/share`, `%LOCALAPPDATA%`, etc.).
    - Environment overrides for non-standard installations and CI.

## Testing
- Lattice’s own test strategy:
  - **Unit tests**:
    - Project descriptor parsing and validation.
    - Dependency resolution (including conflicting versions and overrides).
    - Build graph construction and caching behaviour.
  - **Integration tests**:
    - End-to-end `lattice build/run/test` flows against small Bloch projects.
    - Cross-platform tests for stdlib resolution and user-local paths.
  - **Contract tests with `blochc` and `bloch`**:
    - Verify Lattice is compatible across compiler/runtime versions within a supported range.
    - Ensure error propagation and diagnostics are surfaced clearly to users.

## Compatability
- **With v1.x**
  - Lattice is **not** required for v1.x, but:
    - Compiler CLI contracts introduced in late 1.x are designed so that adoption of Lattice in 2.0.0 does not break existing scripts.
    - Users can continue to call `blochc`/`bloch` directly even after Lattice is introduced.
- **With v2.x and beyond**
  - Lattice is the recommended way to:
    - Manage dependencies.
    - Orchestrate builds and tests.
    - Integrate with registries and package ecosystems.
  - The language remains usable without Lattice for:
    - Small, single-file experiments.
    - Low-level or embedded scenarios where a build system is undesirable.

