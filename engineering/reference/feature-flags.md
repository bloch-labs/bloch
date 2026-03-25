# Feature Flags
Source: `src/bloch/support/feature_flags.hpp`

Inputs:
- Compile-time macros `BLOCH_FEATURE_*` defined by the build system

Outputs:
- constexpr registry of known flags
- Helpers: `is_enabled`, `enabled_flags`, `disabled_flags`, `BLOCH_FEATURE_ENABLED` macro

Behaviour:
Feature flags are gathered into a constexpr registry that exposes helpers to query enabled or disabled flags. The current build recognises `BLOCH_CLASS_SYSTEM` and defaults it to off unless set at compile time. The API keeps lookups cheap and header-only for use throughout the codebase.

Invariants/Guarantees:
- Unknown flags queried via `is_enabled` return false.
- Registry data is constexpr and safe for use in headers.

Edge Cases/Errors:
- None; misuse at compile-time leads to predictable false/zero values.

Extension points:
- Add new entries to `detail::registry` and expose friendly names.
- Consider gating larger features at compile time by wrapping code in `#if BLOCH_FEATURE_ENABLED(...)`.

Tests:
- None dedicated; feature flags are exercised implicitly by builds configured with/without flags.

Related:
- `cli.md`, `runtime-evaluator.md`, `semantics.md`
