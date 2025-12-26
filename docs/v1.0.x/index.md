---
title: Bloch v1.0.x Overview
---

Bloch v1.0.x is the first stable line of the language. It delivers the hybrid programming model, tracked qubits, and simulator-backed execution that later 1.1.x builds on.

## What’s in 1.0.x
- Strongly typed hybrid language with `@quantum` helpers, tracked qubits, and multi-shot execution.
- Core gate set (H, X, Y, Z, rotations, CX) plus measurement/reset semantics enforced at runtime.
- Straightforward CLI for running programs, emitting QASM, and aggregating results.

## How to use these docs
- **Getting Started**: install and run your first program (Linux/macOS/Windows).
- **Language Tour**: full syntax and semantics for the 1.0.x surface.
- **Features**: conceptual overview of the runtime model and design goals.
- **Tooling & CLI**: commands, flags, outputs, and build/test shortcuts.
- **Reference**: per-type pages (under `reference/`) plus gates, CLI, and output formats.
- **Contribute**: workflow, coding standards, testing, and release guidance for the 1.0.x maintenance line.

## Scope of 1.0.x
- Functional programming style only; no class/import syntax.
- Core gate set, measurement/reset semantics, tracked outcomes, and simulator-backed execution.
- Diagnostics focus on correctness and clarity while keeping the language surface stable for the first release.

Use this set when targeting the stable 1.0.x line or backporting fixes.
