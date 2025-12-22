# Release & Versioning

How Bloch releases are tagged and how docs are versioned.

## Release cadence and branching
- Tags follow `vMAJOR.MINOR.PATCH` (e.g., `v1.0.2`).
- Bugfixes may be backported to the latest minor and tagged separately.

## Tagging and changelog
- Update `CHANGELOG.md` with user-visible changes for the release.
- Tag the commit: `git tag v1.0.2 && git push origin v1.0.2`.

## Docs versioning
- Docs use `sphinx-multiversion` to build versions for `v1.0.0`, `v1.0.1`, and `v1.0.2` (`latest`).
- Add a new docs version by tagging a release; update `smv_tag_whitelist` and `smv_latest_version` in `docs/source/conf.py` when a new tag ships.
- Local multi-version build:
  ```bash
  cd docs
  poetry run make multiversion
  poetry run python -m http.server -d _build/html 8000
  ```

## Publishing
- CI should build docs on pushes and tags, then deploy to Firebase for `docs.bloch-labs.com`.
- Ensure the version switcher is present in the deployed site.

## Backports and support
- For critical fixes to older tags, backport the change, retag (e.g., `v1.0.3`), and rebuild/publish docs with the new tag added to the whitelist.
