# Coding Standards & Testing

Expectations for code quality and verification.

## Style
- Follow the repository `.clang-format` for C++.
- Prefer clear naming and small, focused functions; keep headers minimal.
- Document non-obvious logic with brief comments (avoid restating code).

## Static checks
- Enable compiler warnings locally (`-DCMAKE_BUILD_TYPE=Debug` or add `-Wall -Wextra` via toolchain).
- Run any linters or analyzers specific to your platform; surface issues in the PR description.

## Testing
- Add unit or integration tests when changing compiler/runtime behavior.
- Use `ctest --test-dir build --output-on-failure` before submitting.
- For new language surface or behaviors, add a runnable example and link it from {doc}`../../examples/index`.

## Docs
- Update user docs for user-visible changes.
- Note release-impacting changes in the changelog when tagging (see {doc}`release`).
