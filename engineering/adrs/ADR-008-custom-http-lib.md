# ADR-008: In-house HTTP Client

## Owner
Akshay Pal

## Status
Proposed

## Delivered In
Planned for v1.2.0

## Context
- The updater currently depends on `cpp-httplib` + OpenSSL, which drags transitive deps into all binaries, complicates static builds, and expands the attack surface.
- Upcoming networked tools (package manager, remote execution) need a stable, auditable HTTP surface we control.
- Bloch distributions should keep their footprint small and avoid relying on system OpenSSL availability for plain HTTP.

## Decision
- Build a first-party HTTP client library `bloch::http` with a minimal synchronous API (GET/POST, headers, timeout) and pluggable transports.
- Default backend: sockets-based HTTP/1.1 client implemented in-tree (no third-party deps).
- Optional backends: TLS via OpenSSL/Schannel; temporary fallback adapter over `cpp-httplib` until the sockets backend reaches parity.
- Expose the library as an installable target (`bloch_http`) and migrate `bloch_update` to consume it once stable.

## Alternatives Considered
- Keep `cpp-httplib` as the only client — rejected due to dependency and supply-chain exposure.
- Switch to libcurl — rejected for footprint, build complexity, and licensing surface.
- Use platform-native APIs only (WinHTTP, NSURLSession) — rejected for higher divergence and maintenance cost; kept as potential transport plugins.
- Vendor another header-only client (e.g., restclient-cpp) — rejected; still third-party and does not reduce supply-chain risk.

## Consequences
- Pros: smaller default binary, clearer dependency boundaries, easier static packaging, and testable transport seam.
- Cons/Risks: engineering cost to implement and maintain HTTP + TLS correctly; initial feature set limited to HTTP/1.1 and sync calls; need robust cert validation before defaulting to HTTPS.
