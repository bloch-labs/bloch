# DES-004: Stdlib - Boxing

## Scope
* ADR-006

## High Level Flow
- Parser unchanged → Semantics unchanged → Runtime unchanged → Stdlib provides boxed classes with explicit constructors/accessors.

## Compilation (Lexer/Parser/AST)
- No compiler changes.

## Semantic Analysis
- No semantic analysis changes.

## Runtime Architecture
- No runtime changes.

## Stdlib Classes & APIs
- **Object**
  - ctor: `Object()`
  - virtual `equals(Object) -> boolean` (reference equality default)
  - virtual `toPrimitive() -> void` placeholder

- **Boolean**
  - `of(boolean) -> Boolean`
  - `toPrimitive() -> boolean`
  - `equals(Object) -> boolean` (value-based)
  - `toString() -> string` ("true"/"false")
  - `and(Boolean) -> Boolean`
  - `or(Boolean) -> Boolean`
  - `xor(Boolean) -> Boolean`
  - `not() -> Boolean`
  - `static parse(string) -> Boolean`

- **Integer**
  - `of(int) -> Integer`
  - `toPrimitive() -> int`
  - `equals(Object) -> boolean` (value-based)
  - `toString() -> string`
  - `toHexString() -> string`
  - `toBinaryString() -> string`
  - `abs() -> Integer`
  - `negate() -> Integer`
  - `static parse(string) -> Integer`
  - `static min(Integer, Integer) -> Integer`
  - `static max(Integer, Integer) -> Integer`

- **Long**
  - `of(long) -> Long`
  - `toPrimitive() -> long`
  - `equals(Object) -> boolean`
  - `toString() -> string`
  - `toHexString() -> string`
  - `toBinaryString() -> string`
  - `abs() -> Long`
  - `negate() -> Long`
  - `static parse(string) -> Long`
  - `static min(Long, Long) -> Long`
  - `static max(Long, Long) -> Long`

- **Float**
  - `of(float) -> Float`
  - `toPrimitive() -> float`
  - `equals(Object) -> boolean`
  - `toString() -> string`
  - `abs() -> Float`
  - `negate() -> Float`
  - `floor() -> Float`
  - `ceil() -> Float`
  - `round() -> Integer`
  - `static parse(string) -> Float`
  - `static min(Float, Float) -> Float`
  - `static max(Float, Float) -> Float`

- **Char**
  - `of(char) -> Char`
  - `toPrimitive() -> char`
  - `equals(Object) -> boolean`
  - `toString() -> string` (single-character)
  - `isDigit() -> boolean`
  - `isLetter() -> boolean`
  - `isWhitespace() -> boolean`
  - `toUpper() -> Char`
  - `toLower() -> Char`

- **String**
  - `of(string) -> String`
  - `toPrimitive() -> string`
  - `equals(Object) -> boolean` (value-based)
  - `toString() -> string`
  - `length() -> int`
  - `isEmpty() -> boolean`
  - `concat(String) -> String`
  - `substring(int start, int end) -> String`
  - `toUpper() -> String`
  - `toLower() -> String`
  - `trim() -> String`
  - `startsWith(String) -> boolean`
  - `endsWith(String) -> boolean`
  - `static join(String sep, String[] parts) -> String`

## Testing
- Unit tests per class.

## Compatability
- Explicit boxing/unboxing; no auto-boxing. `bit`/`qubit` remain unboxed. Boxes use standard class rules.
