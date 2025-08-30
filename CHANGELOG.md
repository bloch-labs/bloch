# Changelog

All notable changes to the Bloch langugae are documented here

This project follows [Semantic Versioning](https://semver.org/) and the [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) format. 

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

### Changed
- #3: prefix private class members with `m_`
- #2: refactor to allow bloch-lang comments to start with `//`
- #16: refactor parser to remove redundant code
- #34 optimise the Lexer and Parser interfaces 
- #31: use `std::move` to prevent dangling references in the Parser
- #77: simplifed Parser by making better use of the `expect` function
- #107: require `b` suffix for `bit` literals to disambiguate from `int`
- #116: refactored error reporting for better user experience

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

### Removed  
- #24: remove old `version` starter code
- #74: removed `@state` annotations as they are not supported by OpenQASM
- #80: removed `logical<>` primitive type as not MVP
- #75: removed class functionality as not MVP
- #95: removed cpp codegen, Bloch is now fully interpreted
- #111: removed `import` logic, not MVP
- #112: removed `@adjoint` as not MVP











