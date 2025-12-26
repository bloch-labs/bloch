---
title: string
---

UTF-8 string literal enclosed in double quotes.

## Literals
- `"hello"`, `"Bloch 1.0.x"`
- Escape sequences are not supported in 1.0.x (e.g., `"line\n"` is invalid).
- Strings may span lines; content is taken verbatim.

## Usage
- Declarations: `string name = "bloch";`
- Arrays: `string[] words;` (no `string[]` literals beyond `{}`).
- Concatenation: `+` when either operand is a string (`"shots: " + shots`).
- Comparisons: equality/inequality; ordering is not defined.

## Errors
- Unterminated strings are lexical errors.
- Out-of-bounds indexing on `string[]` is a runtime error.
