# Changelog

## [1.1.0](https://github.com/bloch-labs/bloch/compare/v1.0.2...v1.1.0) (2026-01-13)


### Features

* add [@shots](https://github.com/shots) annotation ([#212](https://github.com/bloch-labs/bloch/issues/212)) ([7bb0d77](https://github.com/bloch-labs/bloch/commit/7bb0d77e8d426d612c3b37c9756042c817c039be))
* add class system keywords ([0ef9016](https://github.com/bloch-labs/bloch/commit/0ef9016ffa2d45751fbd6f0193e0b6ab2c04c34a))
* add parser logic for class system ([#198](https://github.com/bloch-labs/bloch/issues/198)) ([654a851](https://github.com/bloch-labs/bloch/commit/654a851c0ef6c75716721a06e978fa4937199234))
* add semantic analysis for class system ([#200](https://github.com/bloch-labs/bloch/issues/200)) ([1b85c1d](https://github.com/bloch-labs/bloch/commit/1b85c1d6bf312a52f4ce16c112a13d1deb4c93c2))
* added runtime support for class system ([#209](https://github.com/bloch-labs/bloch/issues/209)) ([33a7b9c](https://github.com/bloch-labs/bloch/commit/33a7b9c46811eee2e19990ca9e081f46ed5cf7ed))
* explicit casting for numeric values ([#214](https://github.com/bloch-labs/bloch/issues/214)) ([3a199a9](https://github.com/bloch-labs/bloch/commit/3a199a941a9dfc2ff998f6f801130691ae757326))
* import system for class logic ([#210](https://github.com/bloch-labs/bloch/issues/210)) ([7acf8db](https://github.com/bloch-labs/bloch/commit/7acf8db689a7787ae053becb17ad8400c00a598f))
* measure qubit arrays ([#215](https://github.com/bloch-labs/bloch/issues/215)) ([905ee98](https://github.com/bloch-labs/bloch/commit/905ee98b56a9be468de232e5cc7d1211674016bd))


### Build System

* reorg app arch ([#195](https://github.com/bloch-labs/bloch/issues/195)) ([4237004](https://github.com/bloch-labs/bloch/commit/42370042eeef5397c4fadd74898546d8d0b2875f))


### CI/CD

* hotfix packager ([#191](https://github.com/bloch-labs/bloch/issues/191)) ([7ff2058](https://github.com/bloch-labs/bloch/commit/7ff205839de3b1d2e6b8e9aa1e603ca88e7a1975))
* hotfix packager ([#192](https://github.com/bloch-labs/bloch/issues/192)) ([352378c](https://github.com/bloch-labs/bloch/commit/352378cac83b39336908fa112a6c407037d802b7))

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
