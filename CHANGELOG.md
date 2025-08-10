# Changelog

All notable changes to the Bloch langugae are documented here

This project follows [Semantic Versioning](https://semver.org/) and the [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) format. 

## [Unreleased]
### Fixed
- #51: ensured all boolean fields in AST nodes are initialised

## [0.4.2-alpha] - 08/08/2025
### Added
- #53: added language support for `==` and `!=` operators
- #68: added first pass of quantum simulator
- #67: added runtime evalutor which calls the simulator
- #71: added codegen and main cli functionality

## [0.4.2-alpha] - 18/07/2025
### Fixed
- #60: fix update changelog on tag github action 

## [0.4.1-alpha] - 17/07/2025
### Added 
- #56: added a changelog
- #57: added github actions to force changelog entry on PR to `master` and to update changelog on tag 

## [0.4.0-alpha] - 17/07/2025
### Fixed
- #40: fix bug where semantic analysis was not recognising defined functions

### Added
- #44: implement built-in gate functions
- #19: add example `.bloch` files
- #48: implement a type system

## [0.3.1-alpha] - 09/07/2025
### Added
- #20: added Apache License
- #21 Create docs for compiler architecture and grammar

### Changed
- #34 Optimise the Lexer and Parser interfaces by @akshaypal123 in #35
- #31: use `std::move` to prevent dangling references in the Parser by @akshaypal123 in #37

## [0.3.0-alpha] - 02/07/2025
### Added
- #18: initial semantic analysis implementation by @akshaypal123 in #25
- #22: implement else statements by @akshaypal123 in #26
- #23: add keyword to lexer by @akshaypal123 in #27

### Removed  
- #24: remove old `version` starter code by @akshaypal123 in #28

## [0.2.1-alpha] - 01/07/2025
### Changed
- #16: refactor parser to remove redundant code

## [0.2.0-alpha] - 01/07/2025
### Added
- #8: initial AST implementation 
- #14: inital parser implementation

## [0.1.2-alpha] - 30/06/2025
### Added
- #6: added readme
- #7: add contributing.md and formatting to ci pipeline

### Fixed
- #11: minor readme bugfixes

## [0.1.1-alpha] - 30/06/2025
### Changed
- #3: prefix private class members with `m_`
- #2: refactor to allow bloch-lang comments to start with `//`









