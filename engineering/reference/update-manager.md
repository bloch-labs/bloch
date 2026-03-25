# Update Manager
Source: `src/bloch/update/update_manager.hpp`, `src/bloch/update/update_manager.cpp`

Inputs:
- Current version string from CLI (`BLOCH_VERSION`)
- Executable path (`argv0`) for self-update
- Environment: `BLOCH_NO_UPDATE_CHECK`, `BLOCH_OFFLINE`, `CI` to disable checks

Outputs:
- Console notices when newer releases exist
- Updated binary in place on successful `--update`
- On-disk cache file (platform-appropriate cache dir) for version metadata

Behaviour:
`checkForUpdatesIfDue` is a rate-limited probe (every 72h) that fetches the latest GitHub release tag and emits a one-line notice when a newer version exists, skipping entirely in CI/offline/opt-out environments. `performSelfUpdate` downloads the platform-specific archive, optionally verifies checksums, extracts it, and replaces the running binary—in PowerShell on Windows and via `tar` on POSIX—prompting before major upgrades. Both paths rely on `cpp-httplib` over HTTPS with OpenSSL and validate HTTP status codes before acting.

Invariants/Guarantees:
- Never downgrades: if current >= latest, exits with success notice.
- Leaves existing binary untouched on download/verify failures.

Edge Cases/Errors:
- Network/HTTP errors print friendly messages and abort updates.
- Unsupported OS/arch combinations reject with a clear error.

Extension points:
- Add new platforms by extending `osLabel/archLabel` and download asset naming.
- Adjust rate-limiting or cache location by modifying `kUpdateWindow`/`cacheFilePath`.

Tests:
- Exercised indirectly via CLI integration tests; network calls are not mocked in current suite.

Related:
- `cli.md`, `support-and-errors.md`
