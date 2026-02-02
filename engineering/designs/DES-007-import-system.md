# DES-007: Import and Package System

## Scope
* ADR-001
* ADR-009

## High Level Flow
- The import system wires into the existing compilation pipeline by:
  - Recording optional `package` declarations and `import` lists per file during parsing.
  - Loading imported modules **before semantic analysis** via `ModuleLoader`.
  - Validating that imported modules declare the expected `package`.
  - Exposing the merged program to semantic analysis and the runtime.

Conceptual flow:
- Source file → Lexer → Parser (package/imports + declarations) → Module loader (load imports) →
  Semantic analyser → Runtime (no additional import logic; works over merged AST).

## Compilation (Lexer/Parser/AST)
- **Files**
  - `src/bloch/compiler/lexer/lexer.{hpp,cpp}`
  - `src/bloch/compiler/parser/parser.{hpp,cpp}`
  - `src/bloch/compiler/ast/ast.hpp`
  - `src/bloch/compiler/import/module_loader.{hpp,cpp}`

- **Lexer**
  - Ensure the following tokens are recognised as keywords:
    - `package`
    - `import`
  - No additional lexing complexity is required; `.` is already a token used for member access and qualified names.

- **AST**
  - Extend the **file-level AST** node (`Program`) to include:
    - `std::unique_ptr<PackageDeclaration> packageDecl;`
    - `std::vector<std::unique_ptr<ImportDeclaration>> imports;`
  - Structures:
    - `PackageDeclaration { std::vector<std::string> nameParts; /* e.g. ["foo","bar"] */ }`
    - `ImportDeclaration {`
    - `  std::vector<std::string> packageParts; // e.g. ["foo","bar"]`
    - `  std::optional<std::string> symbol;     // e.g. "Baz" if single-type import`
    - `  bool isWildcard;                       // true for import foo.bar.*;`
    - `}`
  - Reuse `NamedType` from the class system design for types referenced in imports where appropriate.

- **Parser**
  - At the start of each file:
    - Optionally parse a `package` declaration:
      - Grammar (simplified): `PackageDecl := "package" Identifier ( "." Identifier )* ";"`.
      - At most one per file; duplicates are an error.
    - Parse zero or more `import` declarations:
      - Grammar (simplified):
        - `ImportDecl := "import" QualifiedName ";" | "import" QualifiedName "." "*" ";"`.
        - `QualifiedName := Identifier ( "." Identifier )*`.
      - Disallow `import` after top-level declarations (for v1.1.0) to keep the file structure simple.
  - Attach the parsed `packageDecl` and `imports` to the file-level AST node.
  - For entrypoint-like files that omit a package (default package), `packageDecl` is left empty.
  - The `ModuleLoader` enforces that **imported modules** declare a `package` matching the
    import path (`foo.bar` → `foo/bar`), while entry files may remain in the default package.

## Semantic Analysis
- **Files**
  - `src/bloch/compiler/semantics/semantic_analyser.{hpp,cpp}`
  - `src/bloch/compiler/import/module_loader.{hpp,cpp}`

- **Current behavior (v1.1.x)**
  - `ModuleLoader` resolves and loads imports before semantic analysis.
  - The semantic analyser operates on a **merged AST** and does not use `ImportDeclaration`
    for name resolution.
  - Names are effectively global; duplicate class names across packages are reported as errors.
  - Package declarations are kept for diagnostics and to validate filesystem layout.

- **Planned behavior (future ADR/DES)**
  - Introduce a package-level symbol table and import scoping rules
    (explicit vs wildcard, local vs imported).

## Runtime Architecture
- **Files**
  - `src/bloch/runtime/runtime_evaluator.{hpp,cpp}`
  - `src/bloch/compiler/import/module_loader.{hpp,cpp}`

- **Runtime impact**
  - The runtime (e.g. `RuntimeClass`, `RuntimeTypeInfo`) does **not** need to understand packages or imports directly.
  - Once semantic analysis has completed, the runtime executes the merged program; no
    additional import logic is required at runtime.

- **ModuleLoader responsibilities**
  - Resolve imports to on-disk paths using the package mapping (`foo.bar` → `foo/bar`).
  - Search order for imports:
    1. Directory of the importing file.
    2. Configured search paths (used by tests/embedding).
    3. Current working directory.
  - For `import foo.bar.Baz;` load `foo/bar/Baz.bloch`.
  - For `import foo.bar.*;` load all `.bloch` files in `foo/bar/`.
  - Validate that each imported module declares `package foo.bar;` (or no `package` for the default package).
  - Detect import cycles, cache loaded modules, and enforce a single `main()` across the graph.

- **Stdlib layout (planned)**
  - Stdlib discovery is not yet implemented in v1.1.x.
  - The import syntax is stable and does not expose the underlying directory structure; once a
    stdlib root is introduced, `ModuleLoader` will extend its search paths without changing
    user-facing imports.

## Testing
- **Unit tests (compiler)**
  - Parser tests:
    - Files with and without `package` declarations.
    - Multiple `import` forms, wildcard imports, and order of declarations.
    - Rejection of multiple package declarations per file.
  - Semantic tests:
    - Duplicate class names across imports are reported as errors.
    - Package declarations are optional on entry files (default package).

- **Unit tests (ModuleLoader)**
  - Pure C++ tests for the module loader logic with a fake filesystem layout:
    - Resolution from the importing file's directory.
    - Resolution from configured search paths.
    - Wildcard imports that load all `.bloch` files in a package directory.
    - Package mismatch errors when an imported module declares the wrong package.
    - Import cycle detection and multiple-main errors.

- **Integration tests**
  - Example programs that:
    - Use `package` and `import` to structure code over multiple files.
    - Omit `package` in a `main.bloch`-style entrypoint and still import from packaged code.
  - Ensure diagnostics are clear when imports fail (e.g. missing module, package mismatch).

## Compatability
- **Backwards compatibility**
  - Existing single-file programs without `package` declarations remain valid as default-package files.
  - Projects that previously relied on path-based multi-file loading are normalised into the
    package/import model; import syntax is additive.
  - The in-repo `stdlib/` directory can be added via search paths during development.

- **Forward compatibility**
  - The import system is intentionally **agnostic** to the underlying library distribution mechanism:
    - Today: local project paths only (no installed stdlib resolution in v1.1.x).
    - Future: Maven-like repository and package manager that expose the same logical packages to the compiler.
  - By treating `ModuleLoader` and a small "library provider" abstraction as the only components aware of on-disk layout, we can:
    - Swap in a `.m2`-like structure without changing language syntax.
    - Introduce library versioning and third-party packages under new ADRs, while preserving the v1.1.0 import semantics.
