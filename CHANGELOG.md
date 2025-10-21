# Changelog

All notable changes to the Bloch programming language are documented here

This project follows [Semantic Versioning](https://semver.org/) and the [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) format. 

## 1.0.0 (2025-10-21)

## What's Changed
* Added lexer and lexer tests by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/1
* #3: prefix private class members with m_ by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/4
* #2: refactor to allow bloch-lang comments to start with '//' by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/5
* #6: added readme by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/9
* #7: add contributing.md and formatting to ci pipeline by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/10
* #11: minor readme bugfixes by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/12
* #8: initial AST implementation by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/13
* #14: inital parser implementation by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/15
* #16: refactor parser to remove redundant code by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/17
* #18: initial semantic analysis implementation by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/25
* #22: implement else statements by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/26
* #23: add  keyword to lexer by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/27
* #24: remove old `version` starter code by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/28
* Optimise the Lexer and Parser interfaces by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/35
* #20: added Apache License by @krrishkk in https://github.com/bloch-labs/bloch/pull/30
* Create docs for compiler architecture and grammar by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/36
* #31: use std::move to prevent dangling references in the Parser by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/37
* #40: fix bug where semantic analysis was not recognising defined functions by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/46
* #44: implement built-in gate functions by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/47
* #19: add example .bloch files by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/49
* #48: implement a type system by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/50
* Update issue templates by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/52
* #56: added a changelog by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/58
* #57: added github actions to force changelog update on PR to master and to update changelog on tag by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/59
* #60: fix update changelog on tag script by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/61
* #60: fix update changelog on tag script by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/62
* #60: fix update changelog on tag script [no-changelog] by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/63
* #60: fix update changelog on tag script [no-changelog] by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/64
* #60: fix update changelog on tag script [no-changelog] by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/65
* #53: add equality and non-equality operators by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/66
* #68: added first pass of quantum simulator by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/69
* #67: added first implementation of bloch runtime evaluator by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/70
* #67: added first implementation of bloch runtime evaluator by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/72
* #71: first pass of codegen and cli functionality by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/73
* #51: ensured all boolean fields in AST nodes are initialised by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/83
* #77: addressed Parser and Lexer warnings and simplified some Parser logic by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/84
* #74: removed @state annotation as not supported by compile target by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/85
* #79: fix division by zero bug by @hamzaqureshi5 in https://github.com/bloch-labs/bloch/pull/82
* #80: removed logical primitive type by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/86
* #81: add integration tests by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/88
* #87: fix bug where echo was outputting to the console twice by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/89
* #75: removed old class functionality as not MVP by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/90
* #78: fix bug where loops were overwriting previous measurements by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/92
* #95: removed cpp code gen, bloch is now fully interpreted by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/96
* #94: echo now works with all and mixed data types by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/97
* #32: added while loops to Bloch by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/98
* #29: added ternary operator by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/100
* #55: added postfix increment and decrement operators by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/102
* #93: fix bug where sem analysis was not enforcing return statements for non-void functions by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/105
* #101: allow inline multiple variable declaration for qubits by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/106
* #107: require b suffix for bit literals to disambiguate from int by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/108
* #104: add shots CLI and @tracked annotation by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/109
* #103: restrict postfix operators to int data type by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/110
* #111: remove import logic as not MVP by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/113
* #112: removed @adjoint annotation as not MVP by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/115
* #116: refactor bloch errors by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/118
* #76: fix `reset` operation in simulator and runtime by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/119
* #54: added logical and bitwise operators and bit arrays by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/120
* #117: added array support by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/122
* #123: refactor `@tracked` CLI output by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/126
* #124: added --help and --version CLI flags by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/129
* bugfix: fix measured qubit being interactable (#131) by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/134
* bugfix: fix install script (#132) by @akshaypal123 in https://github.com/bloch-labs/bloch/pull/135
* merge release-v1.0.0 back into develop by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/138
* update docs and demo by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/139
* ci: add new gh actions for e2e release flow (#140) by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/142
* ci: add new gh actions for e2e release flow (#140) by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/143
* ci: add new gh actions for e2e release flow (#140) by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/144
* ci: add new gh actions for e2e release flow (#140) by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/146
* docs: update documentation (#147) by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/148
* feat: added feature flag config and class system flag by @bloch-akshay in https://github.com/bloch-labs/bloch/pull/153

## New Contributors
* @akshaypal123 made their first contribution in https://github.com/bloch-labs/bloch/pull/1
* @krrishkk made their first contribution in https://github.com/bloch-labs/bloch/pull/30
* @bloch-akshay made their first contribution in https://github.com/bloch-labs/bloch/pull/83
* @hamzaqureshi5 made their first contribution in https://github.com/bloch-labs/bloch/pull/82

**Full Changelog**: https://github.com/bloch-labs/bloch/commits/v1.0.0

## [Unreleased]
### Added
- #6: added readme
- #7: add contributing.md and formatting to ci pipeline
- #8: initial AST implementation 
- #14: inital parser implementation
- #18: initial semantic analysis implementation 
- #22: implement else statements 
- #23: add keyword to lexer
- #20: added Apache License
- #21 create docs for compiler architecture and grammar
- #44: implement built-in gate functions
- #19: add example `.bloch` files
- #48: implement a type system
- #56: added a changelog
- #57: added github actions to force changelog entry on PR to `master` and to update changelog on tag
- #53: added language support for `==` and `!=` operators
- #68: added first pass of quantum simulator
- #67: added runtime evalutor which calls the simulator
- #71: added codegen and main cli functionality
- #81: added integration tests
- #32: added `while` keyword
- #29: added ternary operator
- #55: added post increment and decrement operators `++` and `--`
- #101: added inline multiple variable declaration for `qubit`
- #104: added `--shots` CLI option and `@tracked` annotation
- #103: added semantic checking to `++` and `--` operators
- #54: added logical and bitwise operators
- #117: added robust array support for all primitive types
- #124: added `--help` and `--version` CLI flags

### Changed
- #3: prefix private class members with `m_`
- #2: refactor to allow bloch-lang comments to start with `//`
- #16: refactor parser to remove redundant code
- #34 optimise the Lexer and Parser interfaces 
- #31: use `std::move` to prevent dangling references in the Parser
- #77: simplifed Parser by making better use of the `expect` function
- #107: require `b` suffix for `bit` literals to disambiguate from `int`
- #116: refactored error reporting for better user experience
- #123: refactored `@tracked` rules and CLI output

### Fixed
- #11: minor readme bugfixes
- #40: fix bug where semantic analysis was not recognising defined functions
- #60: fix update changelog on tag github action
- #51: ensured all boolean fields in AST nodes are initialised
- #77: addressed no return type warnings in lexer and parser
- #79: fix division by zero bug in `RuntimeEvaluator::eval` to throw a `RuntimeError` instead of crashing when divisor is zero
- #87: fix bug where `echo()` was printing to the console twice
- #78: fix bug where looped measurements of the same qubit were overwriting old measurements
- #94: `echo()` now works with all and mixed data types
- #93: ensure semantic analysis enforces non-void functions to end with a return statement
- #76: fixed `reset` operation in runtime and simulator
- #131: fixed qbit behaviour after being measured

### Removed  
- #24: remove old `version` starter code
- #74: removed `@state` annotations as they are not supported by OpenQASM
- #80: removed `logical<>` primitive type as not MVP
- #75: removed class functionality as not MVP
- #95: removed cpp codegen, Bloch is now fully interpreted
- #111: removed `import` logic, not MVP
- #112: removed `@adjoint` as not MVP
