# Changelog

## [1.0.3](https://github.com/bloch-labs/bloch/compare/v1.0.2...v1.0.3) (2026-01-21)


### CI/CD

* allow hotfix in PR title ([2e24b57](https://github.com/bloch-labs/bloch/commit/2e24b575e90550ae21c9b99c41208738189aa111))
* auto-ff main in packager and clarify release flow ([b05f59e](https://github.com/bloch-labs/bloch/commit/b05f59e7774eeb00399465d332db1c0c0a657e41))
* new branching strategy ([48a8ade](https://github.com/bloch-labs/bloch/commit/48a8aded6983147f3fa8ecfccbbc93ba5b6ce80e))

## [1.0.2](https://github.com/bloch-labs/bloch/compare/v1.0.1...v1.0.2) (2025-12-11)


### Bug Fixes

* hotfix windows build ([#189](https://github.com/bloch-labs/bloch/issues/189)) ([148bb3d](https://github.com/bloch-labs/bloch/commit/148bb3d30ea48dd48a7a80047cc9c69e580ffe25))

## [1.0.1](https://github.com/bloch-labs/bloch/compare/v1.0.0...v1.0.1) (2025-12-11)


### Bug Fixes

* cx gate works on subspace where ctrl is 1 only ([#180](https://github.com/bloch-labs/bloch/issues/180)) ([6938c5d](https://github.com/bloch-labs/bloch/commit/6938c5db489ddf2f044d62855355f335059635de))
* fixed bug where arrays could only be initialised by literals ([#185](https://github.com/bloch-labs/bloch/issues/185)) ([4c676f3](https://github.com/bloch-labs/bloch/commit/4c676f3e60bec386cc99f1bbefbbc0cab71c59f3))
* initialise m_state to a single amplitude ([#177](https://github.com/bloch-labs/bloch/issues/177)) ([b6d59e2](https://github.com/bloch-labs/bloch/commit/b6d59e232394be9583aaa4366cf96ba4eea5d4d1))
* install scripts migrated to website, added update checker ([#184](https://github.com/bloch-labs/bloch/issues/184)) ([fee4115](https://github.com/bloch-labs/bloch/commit/fee411539b8778d3cc3550d7df23de1f4d725514))
* optimise qasm logging for better performance ([#178](https://github.com/bloch-labs/bloch/issues/178)) ([a2b9cd4](https://github.com/bloch-labs/bloch/commit/a2b9cd4eb7c56efa215b3cf50675ad8278d618de))
* parenthesised expressions not being evaluated at runtime ([#182](https://github.com/bloch-labs/bloch/issues/182)) ([68c6371](https://github.com/bloch-labs/bloch/commit/68c63719bc430da35315b4bffc9b5472298b2beb))
* **perf:** made runtime evaluator single use to prevent state leakage ([#170](https://github.com/bloch-labs/bloch/issues/170)) ([87afa42](https://github.com/bloch-labs/bloch/commit/87afa428208e60bcada4ab4183399ad94ed4e88d))
* reset now clears measured flag ([#172](https://github.com/bloch-labs/bloch/issues/172)) ([e07f9fd](https://github.com/bloch-labs/bloch/commit/e07f9fdd76c9a20e89ccd025aef92e6ae45e4e9f))
