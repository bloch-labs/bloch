# Coding Standards

Inputs:
- New or edited code across the repo

Outputs:
- Consistent, readable code and diagnostics

Behaviour:
Favour clarity over micro-optimisation and extract helpers when functions mix concerns. Use RAII for ownership and avoid raw allocation except where custom deleters are required for runtime objects. Keep data flow explicit by passing state rather than relying on globals. Surface user-facing failures with `BlochError` that includes category and source location; don’t swallow exceptions. Where concurrency exists, guard shared structures such as the GC heap with mutexes or atomics and keep critical sections short. Namespaces mirror folders, includes are ordered with standard headers before project headers, and new features ship with matching tests and doc updates across `docs/reference` and `docs/user`.

Invariants/Guarantees:
- No non-ASCII in code unless required by existing files.
- CLI output remains single-line per message; echo buffering preserves warning-first ordering.

Edge Cases/Errors:
- Avoid adding concurrency around interpreter execution without locks; GC and interpreter share state.

Extension points:
- When introducing new subsystems, add their own reference doc and update `file-index.md`.
- Align new diagnostics with existing error messaging style.

Tests:
- Enforced socially via reviews; no automated enforcement in-tree.

Related:
- `testing.md`, `file-index.md`
