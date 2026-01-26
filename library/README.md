# Bloch Standard Library (scaffold)

This directory houses the Bloch stdlib. Modules should live under `bloch/` and
be importable via qualified names like `import bloch.lang.Integer` or
`import bloch.util.Math`.

Files in this tree are Bloch source and are:
- Installed to `${CMAKE_INSTALL_PREFIX}/library` for runtime discovery.
- Embedded into the compiler binary when `BLOCH_EMBED_STDLIB` is enabled.

Feel free to add more module folders as the surface area grows (e.g., `io`,
`quantum`, `testing`). Keep modules free of `main()` and validate them through
the normal front-end pipeline.
