# Errors & Diagnostics

The Bloch interpreter reports errors with 1-based line and column numbers. It exits with a non-zero code on lexical, parse, semantic, or runtime errors.

## Reading error messages

Errors are displayed in this form:

```
[ERROR]: Stopping program execution...
<ErrorType> error at line:column: error message
```

- **`[ERROR]: Stopping program execution...`** — Indicates the run failed.
- **`<ErrorType>`** — One of: lexical, parse, semantic, or runtime.
- **`line:column`** — Location in the source file (1-based). Use this to jump to the problem.
- **`error message`** — Short description of what went wrong.

Use the line and column to find the exact token or statement; the message often tells you what to fix (e.g. type mismatch, unknown identifier, gate after measurement).

## Error types

### Lexical errors

Reported when the lexer encounters an invalid character or malformed literal (e.g. bad number, unterminated string).

### Parse errors

Reported when the source does not conform to the grammar (e.g. missing semicolon, invalid syntax).

### Semantic errors

Reported by the semantic analyser for type and rule violations (e.g. wrong argument types, duplicate `main()`, invalid annotation use).

### Runtime errors

Thrown during interpretation when executing valid programs that hit invalid states (e.g. invalid cast, division by zero, illegal member access, applying a gate to a qubit after measurement without reset).

## Example error messages

### Parse error

**Output:**

```
[ERROR]: Stopping program execution...
Parse error at 3:1: expected ';' after expression
```

**Cause:** A statement is missing a trailing semicolon on line 3.

**Fix:** Add `;` at the end of the statement on line 3.

### Semantic error (type)

**Output:**

```
[ERROR]: Stopping program execution...
Semantic error at 5:12: cannot assign string to int
```

**Cause:** An expression of type `string` is being assigned to a variable or parameter of type `int` at line 5, column 12.

**Fix:** Use an `int` value or convert/cast appropriately; see [Casting](./casting.md).

### Runtime error (qubit)

**Output:**

```
[ERROR]: Stopping program execution...
Runtime error at 7:5: cannot apply gate to measured qubit without reset
```

**Cause:** A gate (e.g. `h`, `x`, `cx`) was applied to a qubit that has already been measured, without calling `reset` first.

**Fix:** Call `reset q;` before applying more gates to `q`, or use a new qubit. See [Quantum Programming](./quantum-programming.md) and [Troubleshooting](./troubleshooting.md).

## Common errors (quick reference)

| Message pattern | Likely cause | Fix |
|-----------------|--------------|-----|
| expected `;` after expression | Missing semicolon | Add `;` at end of statement. |
| cannot assign … to … | Type mismatch | Use correct type or cast; see [Casting](./casting.md). |
| duplicate definition of main | More than one `main()` across modules | Keep exactly one `main()` in the program. |
| cannot apply gate to measured qubit | Gate used after measure without reset | Call `reset q;` or use a new qubit before more gates. |
| undefined identifier | Typo or use before declaration | Fix name or declare variable before use. |
| division by zero | Runtime divide by 0 | Check divisor before dividing. |
| invalid cast | Unsupported cast (e.g. to `boolean`, `string`) | Only `int`, `long`, `float`, `bit` casts are supported. |

## Tips

- Use small, focused functions and add `echo()` statements to isolate issues.
- For multi-shot runs, add `--echo=all` to see per-shot output alongside tracked summaries.
- Consult the [Language Specification](./language-spec.md) for compile-time and runtime rules.
- See [Troubleshooting](./troubleshooting.md) for more quick checks and [Get help](./troubleshooting.md#get-help).
