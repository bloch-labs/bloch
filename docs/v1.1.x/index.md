---
title: Bloch v1.1.x Overview
---

Bloch v1.1.x is the current line. It keeps the strongly typed, hybrid programming model from 1.0.x and adds full class/import support plus tighter diagnostics.

## What’s in this line
- Strongly typed hybrid runtime: `@quantum` helpers, `@tracked` qubits, measurement/reset rules, multi-shot execution, and OpenQASM emission.
- Class system: single inheritance, visibility, static members, constructors/destructors, `virtual`/`override`, `this`/`super`, and `new`.
- Improved safety: stricter array/type checks, clearer measurement errors, and better diagnostics.
- CLI: `--shots`, `--emit-qasm`, `--echo`, `--update`, `--version`, and the standard help output.

## Who should use 1.1.x
- Teams starting new Bloch projects who want the latest diagnostics and syntax surface.
- Contributors landing changes that prepare for class/import runtime support.
- Readers migrating from 1.0.x; see the Language Tour for the small behavioural differences.

## How to use these docs
- **Getting Started**: install and run your first program (Linux/macOS/Windows).
- **Language Tour**: full syntax and semantics, including lifetime rules and tracked outcomes.
- **Features**: conceptual overview of the runtime model and design goals.
- **Tooling & CLI**: commands, flags, outputs, and build/test shortcuts.
- **Reference**: per-type pages (under `reference/`) plus gates, classes, CLI, and output formats.
- **Contribute**: workflows, coding standards, testing, and release guidance for 1.1.x.
