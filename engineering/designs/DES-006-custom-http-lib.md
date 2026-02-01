# DES-006: In-house HTTP Client

## Scope
* ADR-008

## High Level Flow
`Client` → `Transport` (strategy) → `Socket/TLS adapter` → network → response → `Response` object.  

## Compilation (Lexer/Parser/AST)
- No change.

## Semantic Analysis
- No change.

## Runtime Architecture
- Target: new CMake target `bloch_http` builds from `src/bloch/http/*`.
- Facade: `bloch::http::Client` with `get`/`post`.
- Data types: `Request { method, url, headers, body, timeout_ms }`, `Response { status, headers, body, ok() }`.
- Transport interface (strategy): `Transport::execute(Request) -> Response`.
- Implementations:
  - `SocketsTransport` (default): blocking POSIX/WinSock HTTP/1.1, keep-alive optional.
  - `OpenSSLTransport` (opt-in): wraps sockets with TLS; validates host + cert store.
- Config: backend selected at build; runtime selection possible via an env var/flag if needed later.

## Testing
- Unit tests: transport-independent tests using a loopback harness (send/echo, status codes, headers, timeouts).
- Integration (optional/CI-guarded): hits known httpbin endpoints for real network coverage.
- TLS tests: pinned cert local server to validate hostname verification.

## Compatability
- Platforms: POSIX + WinSock for sockets backend; TLS via OpenSSL initially, Schannel/Darwin Security frameworks considered later.
- HTTP/1.1 only; no proxy or HTTP/2 support initially. Streaming/async deferred to later revisions.
