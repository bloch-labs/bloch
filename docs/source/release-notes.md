# Release Notes

Highlights for the tagged docs versions. See the full [CHANGELOG](https://github.com/bloch-labs/bloch/blob/main/CHANGELOG.md) for complete details.

## v1.0.2 (latest)
- Hotfix for Windows build pipeline to restore release artifacts.

## v1.0.1
- Corrected controlled-X behavior so it fires only when the control qubit is `1`.
- Fixed array initialization to allow non-literal sources.
- Ensured runtime state initializes with a single amplitude.
- Migrated install scripts to the website and added an update checker.
- Optimized QASM logging performance.
- Fixed parenthesized expression evaluation at runtime.
- Made runtime evaluator single use to prevent state leakage.
- `reset` now clears the measured flag.

## v1.0.0
- Initial public release of Bloch with strongly typed quantum/classical integration, compiler, and runtime.
