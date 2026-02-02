# ADR-009: Import and Package System

## Owner
Akshay Pal

## Status
Accepted

## Delivered In
v1.1.0

> Note: stdlib packaging and installation are planned for v1.2.0. The v1.1.x implementation
> resolves imports from local project paths only; stdlib resolution is future work.

## Context
- With the implementation of a class system in Bloch we need to clearly specify an import/package model.
- We want a **Java-style package system** (hierarchical, namespace-based) while **not forcing everything into classes**, closer to C++ where top-level functions and a `main` entry file are idiomatic.
- The import system must:
  - Make it easy to organise projects into packages and modules.
  - Allow "script-style" entrypoints (e.g. a `main.bloch` file without a package).
  - Support a **stdlib** that ships with the language and may later live under a user-local installation root.
  - Be future-proof for additional libraries without painting us into a corner.
- Two concrete deployment options for stdlib surfaced:
  - A **fixed stdlib directory** such as `~/.local/share/bloch/library` that holds one active stdlib version.
  - A **Maven-like repository layout** (e.g. `~/.m2/repository/group/artifact/version/...`) that could host multiple versions and potentially third-party packages.

## Decision
- **Language model**
  - Files may declare an optional **package**:
    - `package foo.bar;`
    - If omitted, the file is in the **default package**, intended for small programs and entrypoints (`main.bloch` style), similar to C++'s `main.cpp`.
  - Files may contain:
    - Top-level functions.
    - Top-level class declarations (per ADR-001).
    - Top-level constant declarations (subject to future ADRs).
  - **Imports**:
    - `import foo.bar.Baz;` imports a single type or symbol from a package.
    - `import foo.bar.*;` imports all symbols from a package (loads all `.bloch` files in that package).
    - `import Baz;` imports a symbol from the default package.
    - `import bloch.collections.List;` and similar forms are reserved for stdlib and future libraries.
  - **Filesystem mapping**
    - `import foo.bar.Baz;` resolves to `foo/bar/Baz.bloch`.
    - Imported modules must declare `package foo.bar;` (or no `package` for the default package).
  - Import resolution is **file-based** in v1.1.x: the compiler maps a fully qualified
    import to a concrete `.bloch` path and loads that module before semantic analysis.
    Packages are used to validate layout rather than to provide namespaces.
  - v1.1.x does **not** provide per-package namespaces; class names must remain unique
    across all loaded modules.

- **Resolution order (v1.1.x)**
  - Given an import `import p.q.R;`, resolution walks:
    1. **Directory of the importing file**.
    2. **Configured search paths** (used by tests and embedding).
    3. **Current working directory**.
  - The importer stops at the first successful resolution; missing imports are reported as semantic errors.
  - For wildcard imports (`import p.q.*;`), the resolver locates the package directory and loads
    all `.bloch` files it contains.

- **Stdlib location (planned for v1.2.0+)**
  - v1.1.x does not yet ship or resolve an installed stdlib; only project paths are searched.
  - Beginning with v1.2.0 (Bloch stdlib release), Bloch will use a **single stdlib root directory**
    per user:
    - Unix-like systems (XDG default): `~/.local/share/bloch/library`.
    - Windows (default): `%LOCALAPPDATA%\\Bloch\\library`.
    - macOS (default): `~/Library/Application Support/Bloch/library`.
    - The actual path is computed via a small platform abstraction and may be overridden via an
      environment variable (e.g. `BLOCH_STDLIB_PATH`) for testing and packaging.
  - Under that root we maintain a **versioned subdirectory**:
    - Example: `~/.local/share/bloch/library/v1.2.0/stdlib/...`
    - The compiler/runtime target a single "active" stdlib version per Bloch binary, matching the
      language version.
  - Package names inside the stdlib are stable and decoupled from the physical layout so that a
    future ADR can introduce a Maven-like repository without changing user-facing import syntax.

- **Future extensibility**
  - The import resolver is structured around a pluggable **search path**:
    - Project roots (always first).
    - One or more **library providers** (stdlib planned for v1.2.0; more in future).
  - A later ADR may:
    - Add a Maven-style multi-version repository for community packages.
    - Allow mapping `group:artifact:version` coordinates onto the same import syntax used for the stdlib.

## Alternatives Considered
- **Alternative 1: Pure Maven-like repository (e.g. `~/.m2`-style) for stdlib**
  - Pros:
    - Naturally supports multiple versions of the stdlib and third-party packages.
    - Aligns with existing tooling mental models (Maven, Gradle, Cargo-style registries).
    - Encourages early design for dependency resolution and caching.
  - Cons:
    - Overkill for v1.1.0, where only a single stdlib version is supported.
    - Introduces configuration (coordinates, repositories, version selection) before there is a user-facing package manager.
    - Complicates bootstrapping and documentation for new users who "just want the language and its stdlib".
  - Outcome:
    - **Deferred**; we design the resolver to allow plugging in a Maven-like backend later without changing import syntax.

- **Alternative 2: Project-only imports (no installed stdlib root)**
  - Pros:
    - Simplest implementation: only load modules from the current project tree (e.g. `src` + `stdlib` dirs in the repo).
    - No need for installation layout or user-local directories.
  - Cons:
    - Not suitable for real installations and package distributions; every project would need a copy of the stdlib.
    - Bloch CLI would not be able to assume a standard stdlib is present on the machine.
  - Outcome:
    - **Rejected**; we want a standard, shared stdlib installation per Bloch version.

- **Alternative 3: Require every file to be in a named package**
  - Pros:
    - Stronger uniformity and clearer namespace hygiene.
    - Closer to "pure Java" semantics.
  - Cons:
    - Makes quick scripts and single-file programs awkward.
    - Unnecessarily diverges from the C++ ergonomics we want for entrypoints (`main.cpp`-like).
  - Outcome:
    - **Rejected**; we support a default package for simplicity and C++-like entry files.

## Consequences
- Projects gain a **clear, documented import and package model** that blends Java-style packages with C++-style entrypoint ergonomics.
- From v1.2.0 onwards, the stdlib is discoverable at a **stable, user-local path** while keeping the door open for future Maven-like dependency resolution without breaking imports.
- Tooling (CLI, editor integrations) can rely on:
  - A well-defined mapping from `package` + symbol to files.
  - A fixed search order (importing file directory → search paths → working directory).
- Future work:
  - Additional ADRs to specify a full package manager, Lattice build system, and Maven-like repository layout.
  - DES-007 (Import System) defines the concrete compiler/runtime architecture and resolver APIs implementing this ADR.
