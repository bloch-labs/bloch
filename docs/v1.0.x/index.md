---
title: Bloch v1.0.x
---

Bloch 1.0.x is the first public line of the Bloch programming language: a strongly typed, functional-first, hybrid classical/quantum language that emits OpenQASM and simulates statevectors out of the box. These docs are written as if 1.0.2 has just shipped—no future features assumed.

Use this guide when you need stability: every construct is documented as an API with examples and the physics underneath so you can design and debug circuits with confidence.

- Start with [Getting Started](getting-started.md) to install the compiler and run your first circuits.
- Deepen with [Language Tour](language-tour.md) for syntax, typing, and quantum flow.
- Understand the model in [Features](features.md) and day-to-day workflows in [Tooling](tooling.md).
- Consult the [Bloch API](bloch-api.md) for exact type, keyword, and gate definitions.
- See how to [Contribute](contribute.md) changes that respect the 1.0.x surface.
- Run and adapt real programs from `examples/` (see `examples/README.md`).

Bloch 1.0.x purposefully excludes classes, imports, and `@shots` annotations. Everything here is functional and explicit so you can focus on quantum correctness and reproducibility.
