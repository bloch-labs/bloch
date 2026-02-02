# Changelog

## [1.1.0](https://github.com/bloch-labs/bloch/compare/v1.0.3...v1.1.0) (2026-02-02)


### Features

* add boolean primitive ([#232](https://github.com/bloch-labs/bloch/issues/232)) ([3bec39d](https://github.com/bloch-labs/bloch/commit/3bec39d64c572c68f898b6f6ae726ab30e97651a))
* add long primitve ([#234](https://github.com/bloch-labs/bloch/issues/234)) ([900ba9d](https://github.com/bloch-labs/bloch/commit/900ba9d4f80f9c59c103fbcdfef4ef88fc013604))
* add null primitive ([#225](https://github.com/bloch-labs/bloch/issues/225)) ([94f8646](https://github.com/bloch-labs/bloch/commit/94f86462dfd4166dc68c44961292eb16d45270be))
* fix class constructor chaining ([#222](https://github.com/bloch-labs/bloch/issues/222)) ([ff9a740](https://github.com/bloch-labs/bloch/commit/ff9a740be2986eedbe4864ce7d25dd817d945419))
* implement generics ([#230](https://github.com/bloch-labs/bloch/issues/230)) ([7e7cd2f](https://github.com/bloch-labs/bloch/commit/7e7cd2f6364ed4b53f3b60321a39e53673cc3f1e))
* implement method overloading ([#226](https://github.com/bloch-labs/bloch/issues/226)) ([d298654](https://github.com/bloch-labs/bloch/commit/d29865474389988237d2f2b15f34f359be794409))


### Performance

* only start gc thread when there are custom classes ([b21b923](https://github.com/bloch-labs/bloch/commit/b21b923e0aa2d09b3952e99262c1fc49d880d004))


### Documentation

* refresh --help cli message ([#223](https://github.com/bloch-labs/bloch/issues/223)) ([ff1f9c9](https://github.com/bloch-labs/bloch/commit/ff1f9c91f693a90a5e5b02daae286e04c770c220))

## [1.0.3](https://github.com/bloch-labs/bloch/compare/v1.0.2...v1.0.3) (2026-01-28)


### Documentation

* adjust figure size for JOSS PDF rendering ([#216](https://github.com/bloch-labs/bloch/issues/216)) ([cecb996](https://github.com/bloch-labs/bloch/commit/cecb9961416e7289e54afab69c1d0563c90a0788))
* joss review feedback ([ed81f8a](https://github.com/bloch-labs/bloch/commit/ed81f8a1cc13f14aaddabd42e1f3dc97d9ad006e))

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
