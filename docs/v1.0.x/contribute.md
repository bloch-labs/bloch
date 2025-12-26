---
title: Contribute
---

How to contribute to the Bloch v1.0.x maintenance line.

## Working model
- Target the maintenance branch (or `develop` if designated); reference an issue for visibility.
- Use Conventional Commits (`<type>(scope): summary`) and mirror the final squash commit in the PR title.
- Keep patches minimal and backward-compatible; prioritise bug fixes, stability, and docs.

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
- Formatting: run `clang-format -i` (project `.clang-format` is canonical).
- Namespaces mirror folders (`bloch::core`, `bloch::runtime`, `bloch::support`, `bloch::update`).
- Strong typing: reject invalid types with clear diagnostics; no silent coercions.
- Qubit lifetime rules: enforce reset before re-measure; warn on unmeasured qubits at scope end.

## Tests & QA
- Add/update tests under `tests/` for parser, semantics, or runtime changes.
- Add regression cases for fixed bugs.
- Document user-visible changes in the matching doc pages (Language Tour, Tooling, Reference).

## Docs
- Keep examples in `examples/` aligned with the docs.
- Update the 1.0.x pages whenever behaviour or CLI changes.

## Releases (manual during 1.0.x)
1. Smoke-test the tag (`ctest`, sample programs with `--shots` and `--emit-qasm`).
2. Run the **Manual Packager** workflow in GitHub Actions for the tag/branch.
3. Download Linux/macOS/Windows archives and checksums; verify hashes.
4. Publish the GitHub release with artifacts, patch notes, and any upgrade guidance.
5. Announce availability; `bloch --update` will pick up the new patch after publish.
