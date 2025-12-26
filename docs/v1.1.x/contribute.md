---
title: Contribute
---

How to contribute to Bloch v1.1.x (code, docs, and releases) in one place.

## Working model
- Target the `develop` branch; keep PRs scoped and reference an issue.
- Use Conventional Commits (`<type>(scope): summary`) and mirror the final squash commit in the PR title.
- Prefer small, reviewable changes; couple behaviour changes with tests and docs updates.

## Environment
- C++20 compiler (GCC ≥ 10, Clang ≥ 10, MSVC 19.28+), CMake ≥ 3.16, Git.
- Optional: `clang-format`, `ccache`. Visual Studio 2022 for Windows builds.

## Build & test
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
Windows: add `-G "Visual Studio 17 2022" -A x64` and `--config Release`.

Smoke tests:
- `./build/bin/bloch examples/02_bell_state.bloch --shots=256`
- `./build/bin/bloch examples/01_hadamard.bloch --emit-qasm`

## Coding standards
- Formatting: run `clang-format -i` (project `.clang-format` is the source of truth).
- Namespaces mirror folders: `bloch::core`, `bloch::runtime`, `bloch::support`, `bloch::update`.
- Strong typing: no silent coercions; reject invalid types with clear diagnostics.
- Qubit lifetime rules: forbid re-measure without reset; warn on unmeasured qubits at scope end.

## Tests & QA
- Add/adjust tests under `tests/` when changing parser, semantics, or runtime behaviour.
- Include regression cases for bugs (parse/semantic/runtime).
- Document user-visible changes in the relevant doc page (Language Tour, Tooling, Reference).

## Docs
- Update the matching versioned doc pages when behaviour or CLI changes.
- Keep examples in `examples/` aligned with the Language Tour and Reference.

## Releases (manual during 1.1.x)
1. Smoke-test the tag (`ctest`, sample programs with `--shots` and `--emit-qasm`).
2. Run the **Manual Packager** workflow in GitHub Actions for the tag/branch.
3. Download Linux/macOS/Windows archives and checksums; verify hashes.
4. Publish the GitHub release with artifacts, changelog highlights, and upgrade notes.
5. Announce availability; `bloch --update` will pick up the new build after publish.
