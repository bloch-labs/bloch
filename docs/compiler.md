# Bloch Compiler Overview

Bloch's compiler transforms source files written in the Bloch language into an intermediate representation suitable for execution or further translation.

## Architecture

1. **Lexer**  
   Converts raw source text into a stream of tokens. The lexer performs whitespace and comment skipping, recognises literals and keywords, and attaches line/column information for diagnostics.
2. **Parser**  
   Consumes the token stream to build an Abstract Syntax Tree (AST). Parsing follows the grammar defined in `grammar.md` and produces nodes such as `Program`, `FunctionDeclaration`, and `Expression` classes located in `src/bloch/ast`.
3. **Semantic Analysis**  
   Traverses the AST to check for correctness. The analyser validates variable declarations, scope rules, function return types and other language constraints. Errors are reported using `BlochRuntimeError` with line and column details.
4. **Code Generation** *(planned)*  
   The current repository does not yet implement backend code generation. The intended target is an OpenQASM compatible representation for running on real or simulated quantum hardware.

## Usage

Building the compiler requires a C++17 toolchain and CMake 3.15 or later:

```bash
mkdir build && cd build
cmake ..
make
```

After building, run the compiler with a Bloch source file:

```bash
./bloch <file.bloch>
```

The compiler will tokenize, parse and analyse the input. When backend code generation is implemented, it will output the resulting program representation.

## Key Files

- `src/bloch/lexer/` – lexical analysis implementation
- `src/bloch/parser/` – recursive descent parser
- `src/bloch/ast/` – AST node definitions
- `src/bloch/semantics/` – semantic analyser

Refer to the source directories for more details on each stage.