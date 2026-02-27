# Changelog

## [1.1.0](https://github.com/bloch-labs/bloch/compare/v1.0.4...v1.1.0) (2026-02-27)


### ⚠ BREAKING CHANGES

* final variables must now be initialised at declaration time or in constructors.

### Features

* add [@shots](https://github.com/shots) annotation ([#212](https://github.com/bloch-labs/bloch/issues/212)) ([7bb0d77](https://github.com/bloch-labs/bloch/commit/7bb0d77e8d426d612c3b37c9756042c817c039be))
* add boolean primitive ([#232](https://github.com/bloch-labs/bloch/issues/232)) ([3bec39d](https://github.com/bloch-labs/bloch/commit/3bec39d64c572c68f898b6f6ae726ab30e97651a))
* add class system keywords ([0ef9016](https://github.com/bloch-labs/bloch/commit/0ef9016ffa2d45751fbd6f0193e0b6ab2c04c34a))
* add long primitve ([#234](https://github.com/bloch-labs/bloch/issues/234)) ([900ba9d](https://github.com/bloch-labs/bloch/commit/900ba9d4f80f9c59c103fbcdfef4ef88fc013604))
* add null primitive ([#225](https://github.com/bloch-labs/bloch/issues/225)) ([94f8646](https://github.com/bloch-labs/bloch/commit/94f86462dfd4166dc68c44961292eb16d45270be))
* add parser logic for class system ([#198](https://github.com/bloch-labs/bloch/issues/198)) ([654a851](https://github.com/bloch-labs/bloch/commit/654a851c0ef6c75716721a06e978fa4937199234))
* add semantic analysis for class system ([#200](https://github.com/bloch-labs/bloch/issues/200)) ([1b85c1d](https://github.com/bloch-labs/bloch/commit/1b85c1d6bf312a52f4ce16c112a13d1deb4c93c2))
* added feature flag config and class system flag ([#153](https://github.com/bloch-labs/bloch/issues/153)) ([baddf23](https://github.com/bloch-labs/bloch/commit/baddf23d087e1a3c4573ce63a7abc2911052dc95))
* added runtime support for class system ([#209](https://github.com/bloch-labs/bloch/issues/209)) ([33a7b9c](https://github.com/bloch-labs/bloch/commit/33a7b9c46811eee2e19990ca9e081f46ed5cf7ed))
* create Object root class ([#238](https://github.com/bloch-labs/bloch/issues/238)) ([e874eb0](https://github.com/bloch-labs/bloch/commit/e874eb0f92140446c7e94072de92f3819057140f))
* enforce final initialiser rule for all variables ([#237](https://github.com/bloch-labs/bloch/issues/237)) ([6869310](https://github.com/bloch-labs/bloch/commit/6869310644cc64b69ecda7acd0d4084dc11866ed))
* explicit casting for numeric values ([#214](https://github.com/bloch-labs/bloch/issues/214)) ([3a199a9](https://github.com/bloch-labs/bloch/commit/3a199a941a9dfc2ff998f6f801130691ae757326))
* fix class constructor chaining ([#222](https://github.com/bloch-labs/bloch/issues/222)) ([ff9a740](https://github.com/bloch-labs/bloch/commit/ff9a740be2986eedbe4864ce7d25dd817d945419))
* implement generics ([#230](https://github.com/bloch-labs/bloch/issues/230)) ([7e7cd2f](https://github.com/bloch-labs/bloch/commit/7e7cd2f6364ed4b53f3b60321a39e53673cc3f1e))
* implement method overloading ([#226](https://github.com/bloch-labs/bloch/issues/226)) ([d298654](https://github.com/bloch-labs/bloch/commit/d29865474389988237d2f2b15f34f359be794409))
* import system for class logic ([#210](https://github.com/bloch-labs/bloch/issues/210)) ([7acf8db](https://github.com/bloch-labs/bloch/commit/7acf8db689a7787ae053becb17ad8400c00a598f))
* measure qubit arrays ([#215](https://github.com/bloch-labs/bloch/issues/215)) ([905ee98](https://github.com/bloch-labs/bloch/commit/905ee98b56a9be468de232e5cc7d1211674016bd))


### Bug Fixes

* cx gate works on subspace where ctrl is 1 only ([#180](https://github.com/bloch-labs/bloch/issues/180)) ([6938c5d](https://github.com/bloch-labs/bloch/commit/6938c5db489ddf2f044d62855355f335059635de))
* fix bloch version command output ([0b50223](https://github.com/bloch-labs/bloch/commit/0b50223f69fde3d737061d965a26f980957af8f3))
* fixed bug where arrays could only be initialised by literals ([#185](https://github.com/bloch-labs/bloch/issues/185)) ([4c676f3](https://github.com/bloch-labs/bloch/commit/4c676f3e60bec386cc99f1bbefbbc0cab71c59f3))
* hotfix windows build ([#189](https://github.com/bloch-labs/bloch/issues/189)) ([148bb3d](https://github.com/bloch-labs/bloch/commit/148bb3d30ea48dd48a7a80047cc9c69e580ffe25))
* initialise m_state to a single amplitude ([#177](https://github.com/bloch-labs/bloch/issues/177)) ([b6d59e2](https://github.com/bloch-labs/bloch/commit/b6d59e232394be9583aaa4366cf96ba4eea5d4d1))
* install scripts migrated to website, added update checker ([#184](https://github.com/bloch-labs/bloch/issues/184)) ([fee4115](https://github.com/bloch-labs/bloch/commit/fee411539b8778d3cc3550d7df23de1f4d725514))
* optimise qasm logging for better performance ([#178](https://github.com/bloch-labs/bloch/issues/178)) ([a2b9cd4](https://github.com/bloch-labs/bloch/commit/a2b9cd4eb7c56efa215b3cf50675ad8278d618de))
* parenthesised expressions not being evaluated at runtime ([#182](https://github.com/bloch-labs/bloch/issues/182)) ([68c6371](https://github.com/bloch-labs/bloch/commit/68c63719bc430da35315b4bffc9b5472298b2beb))
* **perf:** made runtime evaluator single use to prevent state leakage ([#170](https://github.com/bloch-labs/bloch/issues/170)) ([87afa42](https://github.com/bloch-labs/bloch/commit/87afa428208e60bcada4ab4183399ad94ed4e88d))
* reset now clears measured flag ([#172](https://github.com/bloch-labs/bloch/issues/172)) ([e07f9fd](https://github.com/bloch-labs/bloch/commit/e07f9fdd76c9a20e89ccd025aef92e6ae45e4e9f))


### Hot Fixes

* added ps1 install script for Windows ([1ae01ea](https://github.com/bloch-labs/bloch/commit/1ae01ea8af50ab9bf4d7f4778d9b292893e7be87))
* changelog typo ([afd9094](https://github.com/bloch-labs/bloch/commit/afd90942803bc8bd243044b0a9801634f00f4691))
* characters in install.ps1 ([743a002](https://github.com/bloch-labs/bloch/commit/743a00296d3a6303590e4ad2981e0fce1b0ab9ee))
* date format in paper.md ([7975d4c](https://github.com/bloch-labs/bloch/commit/7975d4cecb9c4d7179b52212f245d4d71a2e2685))
* fix install script and manual packager workflow ([019290f](https://github.com/bloch-labs/bloch/commit/019290fab15888e8ac5341fdaf770e3a29abbee8))
* fix manual-packager.yaml workflow ([941d8cd](https://github.com/bloch-labs/bloch/commit/941d8cd748511d51d83e83709678601ff9476c46))
* fix readme and install instructions ([d2d0729](https://github.com/bloch-labs/bloch/commit/d2d0729ae4b991112387ce8ee03eba1cdb586811))
* fix release-please workflow ([0cdbbbb](https://github.com/bloch-labs/bloch/commit/0cdbbbbff9b16b817f89a2fab1c33425bd8056c8))
* github actions for new branch structure ([b815a26](https://github.com/bloch-labs/bloch/commit/b815a260b4a34fd0eb907b1bed0740e64810511c))
* LICENSE, README and NOTICE updates ([3b67296](https://github.com/bloch-labs/bloch/commit/3b67296cfb712d7079eb265465bd62724b101a0c))
* LICENSE, README and NOTICE updates ([56cb74b](https://github.com/bloch-labs/bloch/commit/56cb74b5500fd2954aa308d3252487056aa9c05d))
* prep release v1.0.0 ([6c76e25](https://github.com/bloch-labs/bloch/commit/6c76e252563fdab0f736f244294d6122a3546786))
* prep release v1.0.0 ([bee3a1b](https://github.com/bloch-labs/bloch/commit/bee3a1b92c19db8b4422b695ef0ce07729a456ca))
* prepare for new branching strategy ([4a51e1a](https://github.com/bloch-labs/bloch/commit/4a51e1ae2bac008ed117c5908e9373a63441b6bf))


### Performance

* only start gc thread when there are custom classes ([b21b923](https://github.com/bloch-labs/bloch/commit/b21b923e0aa2d09b3952e99262c1fc49d880d004))


### Documentation

* adjust figure size for JOSS PDF rendering ([#216](https://github.com/bloch-labs/bloch/issues/216)) ([cecb996](https://github.com/bloch-labs/bloch/commit/cecb9961416e7289e54afab69c1d0563c90a0788))
* joss review feedback ([ed81f8a](https://github.com/bloch-labs/bloch/commit/ed81f8a1cc13f14aaddabd42e1f3dc97d9ad006e))
* journal of open source software submission docs ([#156](https://github.com/bloch-labs/bloch/issues/156)) ([2da1abe](https://github.com/bloch-labs/bloch/commit/2da1abee71a7a52758875e8d5893393b5508a9ee))
* refresh --help cli message ([#223](https://github.com/bloch-labs/bloch/issues/223)) ([ff1f9c9](https://github.com/bloch-labs/bloch/commit/ff1f9c91f693a90a5e5b02daae286e04c770c220))
* update contributing.md ([6f36e08](https://github.com/bloch-labs/bloch/commit/6f36e08e091b38310dbe7d1655894ef32b1ecc3c))
* update documentation ([#147](https://github.com/bloch-labs/bloch/issues/147)) ([97c2151](https://github.com/bloch-labs/bloch/commit/97c2151461b26be3d8b495bc4281fd593b070f6b))

## [1.0.4](https://github.com/bloch-labs/bloch/compare/v1.0.3...v1.0.4) (2026-02-21)


### Bug Fixes

* reset analyser state and make scope unwinding exception-safe ([#239](https://github.com/bloch-labs/bloch/issues/239)) ([df13d7a](https://github.com/bloch-labs/bloch/commit/df13d7a7e22ce2aaa89188e5a3f0a44b8f8e3fdb))

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
