# Release Process

This document expands on the automation so maintainers know what to expect during each phase. Use it as the draft for the GitHub wiki release page.

## Preconditions

- `develop` is green and has the desired features.
- CHANGELOG entries are managed by release-please; no manual edits required.
- `VERSION` is bumped automatically in the release-please PR.

## 1. Prepare Release Candidate

Trigger the “Prepare Release Candidate” workflow (Actions → Workflows). Provide `version` only if you need to override the `VERSION` file.

The workflow will:
1. Create or fast-forward the `release-vX.Y.Z` branch from `develop`.
2. Tag `vX.Y.Z-rc.1` off that branch.

The `vX.Y.Z-rc.1` tag immediately kicks off the **Release Candidate Build** workflow:
- Linux + macOS Release builds.
- Tests, packaging, checksum generation.
- Publishes a GitHub pre-release with artifacts attached.

## 2. Iterate on RCs

To fix issues discovered in RCs:

```bash
git checkout release-vX.Y.Z
# apply fixes
git commit -m "fix: ... (#123)"
git tag -a vX.Y.Z-rc.2 -m "RC 2"
git push origin release-vX.Y.Z vX.Y.Z-rc.2
```

Each new `-rc.N` tag repeats the pre-release build and updates the GitHub pre-release with fresh assets.

## 3. Finalise the Release

1. Open a PR from `release-vX.Y.Z` to `master`.
2. CI runs the full Release build matrix and packaging dry-run.
3. Merge the PR when green.

Merging to `master` updates the `VERSION` file. The **Tag Release** workflow automatically tags `vX.Y.Z` and pushes it to origin.

## 4. Publish

The push tag triggers the **Publish Release** workflow:
- Builds Linux + macOS artifacts.
- Generates checksums.
- Uses `scripts/ci/extract_release_notes.py` to grab the changelog section and attaches it as a draft GitHub Release.

Review the draft release notes, tweak as needed, and click “Publish release”.

## 5. Post-Release

- Merge `master` back into `develop` (or fast-forward) if they diverged.
- Communicate the release (blog, social channels).
- Open a milestone for the next iteration.

Future documentation on the wiki will cover common failure modes and manual rollback instructions.
