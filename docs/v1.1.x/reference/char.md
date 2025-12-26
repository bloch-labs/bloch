---
title: char
---

Single character.

## Literals
- `'a'`, `'Z'`, `'0'`
- No escape sequences are supported in 1.1.x (e.g., `'\n'` is invalid).

## Usage
- Declarations: `char c = 'a';`
- Arrays: `char[4] name;`, `char[] dyn;`
- Operators: comparisons; arithmetic/bitwise are not defined on `char`.
- Strings are separate from `char`; concatenate strings only.

## Errors
- Unterminated or multi-character char literals are lexical errors.
- Out-of-bounds indexing on `char[]` is a runtime error.
