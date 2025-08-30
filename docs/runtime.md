# Bloch Runtime Overview

Bloch processes source files written in the Bloch language and executes them directly using an interpreter that can emit OpenQASM for quantum circuits.

## Architecture

1. **Lexer**
   Converts raw source text into a stream of tokens. The lexer performs whitespace and comment skipping, recognises literals and keywords, and attaches line/column information for diagnostics.
2. **Parser**
   Consumes the token stream to build an Abstract Syntax Tree (AST). Parsing follows the grammar defined in `grammar.md` and produces nodes such as `Program`, `FunctionDeclaration`, and `Expression` classes located in `src/bloch/ast`.
3. **Semantic Analysis**
   Traverses the AST to check for correctness. The analyser validates variable declarations, scope rules, function return types and other language constraints. Errors are reported using `BlochError` with line and column details.
4. **Runtime Evaluator**
   The `RuntimeEvaluator` walks the AST to interpret the program, invoking the simulator for quantum operations, collecting measurements, and producing OpenQASM as needed.

## Usage

Building the runtime requires a C++17 toolchain and CMake 3.15 or later:

```bash
mkdir build && cd build
cmake ..
make
```

After building, run Bloch with a `.bloch` source file:

```bash
./bloch <file.bloch>
```

Bloch will tokenise, parse, analyse, and execute the input program. When requested, it outputs the corresponding OpenQASM.

## Key Files

- `src/bloch/lexer/` – lexical analysis implementation
- `src/bloch/parser/` – recursive descent parser
- `src/bloch/ast/` – AST node definitions
- `src/bloch/semantics/` – semantic analyser

Refer to the source directories for more details on each stage.