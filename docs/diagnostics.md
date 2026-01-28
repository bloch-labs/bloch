---
title: Errors & Diagnostics
---
# Errors & Diagnostics

The Bloch interpreter reports errors with 1-based line and column numbers. It exits non-zero on lexical, parse, semantic, or runtime errors.

This page shows typical diagnostics and what they mean.

All errors are displayed in the form 

```
[ERROR]: Stopping program execution...
<ErrorType> error at line:column: error message
```

## Lexical errors

Reported when the lexer encounters an invalid character or malformed literal.

## Parse errors

Reported when the source does not conform to the grammar.

## Semantic errors

Reported by the semantic analyser for type and rule violations.

## Runtime errors

Thrown during interpretation when executing valid programs that hit invalid states.

## Tips

- Use small, focused functions and add `echo()` statements to isolate issues.
- For multi-shot runs, add `--echo=all` to see per-shot output alongside tracked summaries.
- Consult the [Semantic Rules](./language/semantics) for a complete list of compile-time checks.

