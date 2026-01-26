# Lexer
Source: `src/bloch/core/lexer/lexer.hpp`, `src/bloch/core/lexer/lexer.cpp`, `src/bloch/core/lexer/token.hpp`

Inputs:
- Raw source text (`std::string_view`) from a `.bloch` file

Outputs:
- Flat `std::vector<Token>` including a trailing `Eof` token
- `BlochError` on lexical failures with `Lexical` category.

Behaviour:
The lexer runs a single forward pass with minimal lookahead, turning characters into tokens while skipping whitespace and `//` comments. It recognises ints, floats with an `f` suffix, bits restricted to `0b`/`1b`, strings (no escapes yet), and single-character chars. Keywords cover primitives, control flow, the class system, annotations, and built-ins. Line and column are recorded per token, with the column pointing to the token start to make diagnostics precise.

Invariants/Guarantees:
- Always appends exactly one `Eof` token.
- Rejects malformed float literals (missing `f`) and bit literals other than `0b`/`1b`.
- Treats `//` to end-of-line as a comment; no block comments.

Edge Cases/Errors:
- Unterminated string/char throws a `Lexical` `BlochError`.
- Unknown characters emit `Unknown` tokens (parser will surface errors later).

Extension points:
- Add keywords/tokens in `TokenType` and the keyword map in `scanIdentifierOrKeyword`.

Tests:
- `tests/test_lexer.cpp`

Related:
- `parser.md`, `ast.md`, `semantics.md`
