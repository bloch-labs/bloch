# Support & Errors
Source: `src/bloch/support/error/bloch_error.hpp`

Inputs:
- Messages, line/column (when available), and error categories from callers

Outputs:
- Formatted coloured strings for stderr/stdout
- `BlochError` exceptions carrying category, line, column

Behaviour:
This layer formats diagnostics with ANSI colours and consistent prefixes, exposing helpers for info and warning logs and a `BlochError` exception that carries category and source location. Messages reset colours after each line to avoid bleeding styles, and callers pass line/column when available so errors map directly to source.

Invariants/Guarantees:
- Colour codes are reset after each message to avoid leaking styling.
- If line/column are zero, locations are omitted from formatted messages.

Edge Cases/Errors:
- None internally; relies on caller-supplied data. Consumers must avoid throwing with invalid UTF-8 (all code uses ASCII).

Extension points:
- Add new categories to `ErrorCategory` if new phases are introduced; ensure format helpers cover them.
- For non-colour environments, gate colour output behind a feature flag or env check.

Tests:
- Indirect via semantics/runtime/CLI tests that assert on error messages and locations.

Related:
- `semantics.md`, `runtime-evaluator.md`, `cli.md`
