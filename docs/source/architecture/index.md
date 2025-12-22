# Architecture Overview

High-level view of the Bloch compiler, runtime, and supporting tooling.

## Frontend
- **Lexer/Parser:** Tokenization and Pratt parser building the AST (`src/bloch/core/lexer`, `src/bloch/core/parser`).
- **Semantic analysis:** Type checking and validation over the AST.

## Middle/IR
- **IR/passes:** Transformations and checks preparing programs for execution.
- **Diagnostics:** Error reporting and source mapping.

## Runtime and backends
- **Runtime evaluator:** Executes validated programs and manages state (`src/bloch/runtime`).
- **Targets:** Current statevector/OpenQASM simulator support; future hardware backends can hook in here.

## Support and utilities
- **Support library:** Shared helpers, errors, and feature flags (`src/bloch/support`).
- **Updater:** Self-update plumbing for the CLI (`src/bloch/update`).

## Tooling
- **VS Code extension:** Language support and editing experience (see `bloch-labs/bloch-vscode`).
- **Docs:** Sphinx+MyST site in `docs/` with versioned builds.

Add diagrams and deep dives per subsystem as needed.
