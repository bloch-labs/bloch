# Contribution Workflow

How to propose, review, and land changes in Bloch.

## Issues and planning
- Use GitHub issues with clear repros or design notes; label `bug`, `feature`, or `docs`.
- For larger changes, open an RFC/discussion before implementation.

## Branching and commits
- Branch from `main` (or the target release branch when cherry-picking a fix).
- Keep commits scoped and descriptive; prefer imperative commit messages.

## Pull requests
- Link the issue, describe behavior changes, and include screenshots/logs where relevant.
- Update or add tests when you touch runtime/compiler logic.
- Update docs when user-facing behavior changes; keep examples in sync.
- CI should be green (build + tests) before requesting review.

## Reviews
- Be explicit about correctness, performance, and user impact.
- Keep PRs small and focused to speed up review.
- Address feedback with follow-up commits; avoid force-pushing over reviews unless necessary.

## Landing changes
- Merge via squash or rebase per repository policy.
- For fixes needing backports (e.g., to `v1.0.1`), coordinate tags and changelog entries with maintainers.
