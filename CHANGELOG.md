# Changelog

## [1.1.0](https://github.com/bloch-labs/bloch/compare/v1.0.4...v1.1.0) (2026-08-29)


### ⚠ BREAKING CHANGES

* final variables must now be initialised at declaration time or in constructors.

### ⚛️ Features

* add [@shots](https://github.com/shots) annotation ([#212](https://github.com/bloch-labs/bloch/issues/212)) ([ebb33e4](https://github.com/bloch-labs/bloch/commit/ebb33e4a70d136c580c1165a4acd52ce0fe2857e))
* add boolean primitive ([#232](https://github.com/bloch-labs/bloch/issues/232)) ([59a8838](https://github.com/bloch-labs/bloch/commit/59a88382a17d62db8342ca683b759be9aa98381d))
* add class system keywords ([ca1ddfa](https://github.com/bloch-labs/bloch/commit/ca1ddfad6a0a5c59cdacd87dc9fbc515062ce51f))
* add long primitve ([#234](https://github.com/bloch-labs/bloch/issues/234)) ([f9d47af](https://github.com/bloch-labs/bloch/commit/f9d47af122f9645a1ad116b6bae9a5074ecca1e8))
* add null primitive ([#225](https://github.com/bloch-labs/bloch/issues/225)) ([538ae3a](https://github.com/bloch-labs/bloch/commit/538ae3ae20394e72f1b1c7db941a15108b3c8bd7))
* add parser logic for class system ([#198](https://github.com/bloch-labs/bloch/issues/198)) ([f0c21d0](https://github.com/bloch-labs/bloch/commit/f0c21d0ee02017071b0a5ac895bba4fc470b5a15))
* add semantic analysis for class system ([#200](https://github.com/bloch-labs/bloch/issues/200)) ([4e7fcbc](https://github.com/bloch-labs/bloch/commit/4e7fcbce4ba4c3b490851749cc05d5183b0c3dd9))
* added runtime support for class system ([#209](https://github.com/bloch-labs/bloch/issues/209)) ([9940fcc](https://github.com/bloch-labs/bloch/commit/9940fcc0034f8072266708f04ee622588d6c8a0d))
* create Object root class ([#238](https://github.com/bloch-labs/bloch/issues/238)) ([9cf2d5a](https://github.com/bloch-labs/bloch/commit/9cf2d5af5e335a68269dc6be3fdeac7ad74013d6))
* enforce final initialiser rule for all variables ([#237](https://github.com/bloch-labs/bloch/issues/237)) ([cfd54d3](https://github.com/bloch-labs/bloch/commit/cfd54d3e18ac1ad1f9d54fb9ecb2155a0f171eaa))
* explicit casting for numeric values ([#214](https://github.com/bloch-labs/bloch/issues/214)) ([a79fae9](https://github.com/bloch-labs/bloch/commit/a79fae9f73b241667fb074df47f4335af432839f))
* fix class constructor chaining ([#222](https://github.com/bloch-labs/bloch/issues/222)) ([58695c5](https://github.com/bloch-labs/bloch/commit/58695c52fde2b0efb1fe8b9ce7da601eab5871a8))
* implement generics ([#230](https://github.com/bloch-labs/bloch/issues/230)) ([dec471f](https://github.com/bloch-labs/bloch/commit/dec471f5f56c3f127f90e6feba3990cc990aac6f))
* implement method overloading ([#226](https://github.com/bloch-labs/bloch/issues/226)) ([ec05a7d](https://github.com/bloch-labs/bloch/commit/ec05a7d579088969f2c544081849d6b7b650d55a))
* import system for class logic ([#210](https://github.com/bloch-labs/bloch/issues/210)) ([9207005](https://github.com/bloch-labs/bloch/commit/920700544a20ee60b0edf4ca4216d6b34c4c3b31))
* measure qubit arrays ([#215](https://github.com/bloch-labs/bloch/issues/215)) ([b480006](https://github.com/bloch-labs/bloch/commit/b4800068870e3ac25f7f3c604446ef718ee2f68d))


### 🐛 Bug Fixes

* class annotation are now correctly parsed ([#244](https://github.com/bloch-labs/bloch/issues/244)) ([61d53da](https://github.com/bloch-labs/bloch/commit/61d53da32ebc61d65837d3dbc413f57287506b82))
* preserve generic arguments in extends clauses ([#254](https://github.com/bloch-labs/bloch/issues/254)) ([95bab19](https://github.com/bloch-labs/bloch/commit/95bab19619731c61ba122cba16fbebb07aaed11e))
* report errors in custom user destructor logic without crashing ([#255](https://github.com/bloch-labs/bloch/issues/255)) ([1b6bbd2](https://github.com/bloch-labs/bloch/commit/1b6bbd24cab2deb3da92871f97d51437546b71b8))


### ⚡ Performance

* only start gc thread when there are custom classes ([20f7ab0](https://github.com/bloch-labs/bloch/commit/20f7ab089b08f0579568efbc0d0c880b967d8008))

## [1.0.4](https://github.com/bloch-labs/bloch/compare/v1.0.3...v1.0.4) (2026-02-21)


### Bug Fixes

* reset analyser state and make scope unwinding exception-safe ([#239](https://github.com/bloch-labs/bloch/issues/239)) ([df13d7a](https://github.com/bloch-labs/bloch/commit/df13d7a7e22ce2aaa89188e5a3f0a44b8f8e3fdb))

## [1.0.2](https://github.com/bloch-labs/bloch/compare/v1.0.1...v1.0.2) (2025-12-11)


### Bug Fixes

* hotfix windows build ([#189](https://github.com/bloch-labs/bloch/issues/189)) ([148bb3d](https://github.com/bloch-labs/bloch/commit/148bb3d30ea48dd48a7a80047cc9c69e580ffe25))
* cx gate works on subspace where ctrl is 1 only ([#180](https://github.com/bloch-labs/bloch/issues/180)) ([6938c5d](https://github.com/bloch-labs/bloch/commit/6938c5db489ddf2f044d62855355f335059635de))
* fixed bug where arrays could only be initialised by literals ([#185](https://github.com/bloch-labs/bloch/issues/185)) ([4c676f3](https://github.com/bloch-labs/bloch/commit/4c676f3e60bec386cc99f1bbefbbc0cab71c59f3))
* initialise m_state to a single amplitude ([#177](https://github.com/bloch-labs/bloch/issues/177)) ([b6d59e2](https://github.com/bloch-labs/bloch/commit/b6d59e232394be9583aaa4366cf96ba4eea5d4d1))
* install scripts migrated to website, added update checker ([#184](https://github.com/bloch-labs/bloch/issues/184)) ([fee4115](https://github.com/bloch-labs/bloch/commit/fee411539b8778d3cc3550d7df23de1f4d725514))
* optimise qasm logging for better performance ([#178](https://github.com/bloch-labs/bloch/issues/178)) ([a2b9cd4](https://github.com/bloch-labs/bloch/commit/a2b9cd4eb7c56efa215b3cf50675ad8278d618de))
* parenthesised expressions not being evaluated at runtime ([#182](https://github.com/bloch-labs/bloch/issues/182)) ([68c6371](https://github.com/bloch-labs/bloch/commit/68c63719bc430da35315b4bffc9b5472298b2beb))
* **perf:** made runtime evaluator single use to prevent state leakage ([#170](https://github.com/bloch-labs/bloch/issues/170)) ([87afa42](https://github.com/bloch-labs/bloch/commit/87afa428208e60bcada4ab4183399ad94ed4e88d))
* reset now clears measured flag ([#172](https://github.com/bloch-labs/bloch/issues/172)) ([e07f9fd](https://github.com/bloch-labs/bloch/commit/e07f9fdd76c9a20e89ccd025aef92e6ae45e4e9f))
