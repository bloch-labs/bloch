# How to Contribute

Thanks for your interest in Bloch! This guide mirrors the contribution style used by mature
open-source projects: start small, keep changes focused, and communicate early.

## Ways to contribute
- Fix bugs or implement features in the compiler/runtime.
- Improve docs in `docs/` or contributor references here.
- Add or expand tests in `tests/`.
- Triage issues and propose design changes in `engineering/designs/`.

## Before you start
- Check existing issues and discussions for context.
- If you plan a larger change, open an issue or design doc first to align on scope.

## Development setup
```bash
git clone https://github.com/bloch-labs/bloch.git
cd bloch
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run the compiler against an example:
```bash
./build/bin/bloch examples/02_bell_state.bloch --emit-qasm
```

## Workflow
1. Create a feature branch.
2. Make focused changes with tests.
3. Update docs in `docs/` or `engineering/reference/` when behavior changes.
4. Open a PR with a clear summary and test coverage notes.

## PR checklist
- Tests pass locally (use `./scripts/build_and_test.sh`).
- Code is formatted (use `./scripts/format.sh`).
- New behavior is covered by tests.
- Docs updated when user-facing behavior changes.
- Follow the [Coding Standards](./coding-standards.md).

## Where to look next
- [Codebase Overview](./codebase-overview.md)
- [Architecture](./architecture.md)
- [Testing](./testing.md)
