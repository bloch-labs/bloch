---
title: "Bloch: a strongly typed, hardware-agnostic, hybrid quantum programming language"
tags:
  - quantum computing
  - programming languages
  - compilers
  - simulation
authors:
  - name: Akshay Pal
    orcid: 0009-0004-0465-4420
    affiliation: 1
affiliations:
  - name: Independent Researcher, United Kingdom
    index: 1
date: 23 November 2025
bibliography: paper.bib
---

# Summary

Bloch is a modern quantum programming language and interpreter designed to feel
familiar to systems developers while remaining hardware-agnostic. The project
combines a statically typed surface language, a semantic analyser that guards
against classical/quantum misuse before execution, and an interpreter that
emits OpenQASM traces and aggregates multi-shot measurement results. Bloch
supports classical control flow, deterministic resource management, and
first-class annotations such as `@quantum` (to delineate quantum code) and
`@tracked` (to stream measurements into probability tables). The language is
implemented in C++20, ships with a comprehensive test suite, and is licensed
under Apache-2.0 to encourage adoption in research and industrial workflows.

# Statement of Need

Quantum practitioners frequently prototype algorithms in Python-first SDKs like
Qiskit [@qiskit-zenodo] or Cirq [@cirq-zenodo], where dynamic typing and
runtime-only feedback can make it difficult to catch logic errors prior to
simulation or hardware execution. Researchers building higher-level language
abstractions (e.g., Silq [@silq] and Quipper [@quipper]) typically need to choose
between tightly coupled vendor ecosystems or experimenting with new type
systems without a reference interpreter capable of emitting standard assembly
formats (OpenQASM 2 [@openqasm2]). Bloch fills this gap by delivering a
self-contained toolchain that:

* enforces a compact, explicit type system over both classical and quantum data
  so that invalid measurements, illegal `@quantum` return types, and improper
  qubit mutations are rejected at compile time;
* produces OpenQASM output by default, which enables researchers to feed Bloch
  programs into downstream simulators or device backends without rewriting
  kernels; and
* exposes deterministic multi-shot execution with aggregated statistics so that
  algorithm designers can validate probabilistic behaviour before running on expensive
  quantum hardware.

By targeting developers who are comfortable with systems languages but need a
high-level quantum DSL, Bloch reduces the friction between research sketches
and reproducible experiments.

# Design and Implementation

Bloch’s architecture is intentionally modular:

* **Front-end.** A hand-written lexer and Pratt-style parser (under
  `src/bloch/lexer` and `src/bloch/parser`) build an abstract syntax tree (AST)
  that covers declarations, control flow constructs, quantum annotations, and
  array expressions. The AST is shared by the analyser, interpreter, and tests.
* **Static semantics.** The analyser (`src/bloch/semantics`) maintains a scoped
  symbol table and a compact `ValueType` universe to ensure that only valid
  combinations of classical and quantum operations progress to execution. It
  also enforces Bloch-specific rules, such as restricting `@quantum` functions
  to `bit` or `void` return values and rejecting `@tracked` annotations on unsupported
  types.
* **Runtime & simulator.** The interpreter (`src/bloch/runtime`) evaluates the
  AST, orchestrates an ideal statevector simulator, and records measurement
  outcomes per tracked symbol. Every run emits an OpenQASM trace, and the CLI
  (`src/main.cpp`) can execute programs for `N` shots, aggregate measurement
  counts, and emit tabulated probability estimates alongside the generated
  QASM.
* **Feature staging.** The lightweight feature-flag registry
  (`src/bloch/feature_flags.hpp`) gives maintainers a way to land experimental
  language constructs (e.g., an upcoming class system) without destabilising
  the default build.

The project distributes platform-agnostic CMake build files and is validated on
Linux, macOS, and Windows. Users interact with the CLI (`bloch <file.bloch>`)
which offers `--emit-qasm`, `--shots=N`, and `--echo` parameters for reproducible
experiments.

# Quality Control

Bloch ships with unit and integration tests implemented with the project’s
minimal test harness (`tests/test_framework.hpp`). The test suite covers the
entire pipeline:

* lexical analysis and token categorisation (`tests/test_lexer.cpp`),
* parser shape and AST formation (`tests/test_parser.cpp`),
* static semantics (e.g., scope rules, `@quantum` return constraints, `final`
  assignments) in `tests/test_semantics.cpp`,
* runtime behaviour such as OpenQASM emission, measurement persistence inside
  loops, `@tracked` aggregation, and echo handling in `tests/test_runtime.cpp`,
  and
* integration smoke tests that exercise representative Bloch programs end to
  end (`tests/test_integration.cpp`).

Continuous integration executes `ctest` on every pull request, while developers
can repeat the same workflow locally via the commands documented in
`README.md`. Coverage touches the lexer, parser, semantic analyser, and runtime,
providing confidence that regressions in the type system, simulator, or CLI are
caught early.

# Use Cases

Bloch aims to reduce the distance between whiteboard circuits and evaluable
experiments. Current use cases include:

1. **Educational demos.** The Bell-state example (`examples/02_bell_state.bloch`)
   showcases hardware-agnostic entanglement with deterministic statistics
   reporting, making it suitable for classroom explanations or live coding.
2. **Algorithm sketching.** Researchers can iteratively design algorithms that
   mix classical control flow with quantum kernels, then export the emitted
   OpenQASM for downstream tooling without reimplementation.
3. **Runtime experimentation.** The `@tracked` facility and `--shots` flag make
   it straightforward to explore noise-free distributions and verify that
   optimisations preserve measured behaviour before porting kernels to other
   stacks.

# Availability

Bloch is openly developed at https://github.com/bloch-labs/bloch under the
Apache-2.0 license. The repository bundles setup instructions, contribution
guidelines, and a list of feature flags so that new contributors can propose
language extensions while maintaining release stability. Pre-built binaries are
not required; the CMake toolchain builds the CLI across major platforms, and the
project’s documentation hub (https://docs.bloch-labs.com) provides user guides and API
notes.

# Acknowledgements

I thank the Bloch open source community for issue reports, early design feedback, and
preview testing, as well as the maintainers of the upstream projects cited in
this paper whose work makes Bloch interoperability possible.

# References
