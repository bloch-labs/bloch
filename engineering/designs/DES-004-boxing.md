# DES-004: Boxing Strategy in Stdlib

## Scope
Implements ADR-006: boxed classes for every primitive (excluding `bit`/`qubit`). Provides explicit APIs so primitives can be used in generic/reference-only contexts.

## High Level Flow
- Parser unchanged → Semantics unchanged → Runtime unchanged → Stdlib provides boxed classes with explicit constructors/accessors.

## Compilation (Lexer/Parser/AST)
- No compiler changes; boxes are plain classes in stdlib.

## Semantic Analysis
- No new rules; boxes follow normal class rules.

## Runtime Architecture
- No runtime changes; boxes are instantiated like any class.

## Stdlib Classes & APIs
- **Object**
  - ctor: `Object()`
  - virtual `equals(Object) -> boolean` (reference equality default)
  - virtual `toPrimitive() -> void` placeholder

- **Boolean**
  - `valueOf(boolean) -> Boolean`
  - `toPrimitive() -> boolean`
  - `equals(Object) -> boolean` (value-based)
  - `toString() -> string` ("true"/"false")
  - `and(Boolean) -> Boolean`, `or(Boolean) -> Boolean`, `xor(Boolean) -> Boolean`, `not() -> Boolean`
  - `compareTo(Boolean) -> int`
  - `static parse(string) -> Boolean`

- **Integer**
  - `valueOf(int) -> Integer`
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
  - `valueOf(long) -> Long`
  - `toPrimitive() -> long`
  - `equals(Object) -> boolean`
  - `toString() -> string`
  - `toHexString() -> string`
  - `toBinaryString() -> string`
  - `abs() -> Long`
  - `negate() -> Long`
  - `compareTo(Long) -> int`
  - `static parse(string) -> Long`
  - `static min(Long, Long) -> Long`
  - `static max(Long, Long) -> Long`

- **Float**
  - `valueOf(float) -> Float`
  - `toPrimitive() -> float`
  - `equals(Object) -> boolean`
  - `toString() -> string`
  - `abs() -> Float`
  - `negate() -> Float`
  - `floor() -> Float`
  - `ceil() -> Float`
  - `round() -> Integer`
  - `compareTo(Float) -> int`
  - `static parse(string) -> Float`
  - `static min(Float, Float) -> Float`
  - `static max(Float, Float) -> Float`

- **Char**
  - `valueOf(char) -> Char`
  - `toPrimitive() -> char`
  - `equals(Object) -> boolean`
  - `toString() -> string` (single-character)
  - `isDigit() -> boolean`
  - `isLetter() -> boolean`
  - `isWhitespace() -> boolean`
  - `toUpper() -> Char`
  - `toLower() -> Char`
  - `compareTo(Char) -> int`

- **String**
  - `valueOf(string) -> String`
  - `toPrimitive() -> string`
  - `equals(Object) -> boolean` (value-based)
  - `toString() -> string`
  - `length() -> int`
  - `isEmpty() -> boolean`
  - `concat(String) -> String`
  - `substring(int start, int end) -> String`
  - `indexOf(String) -> int`
  - `toUpper() -> String`
  - `toLower() -> String`
  - `trim() -> String`
  - `startsWith(String) -> boolean`
  - `endsWith(String) -> boolean`
  - `compareTo(String) -> int`
  - `static join(String sep, String[] parts) -> String`

## Testing
- Unit tests per class.

## Compatability
- Explicit boxing/unboxing; no auto-boxing. `bit`/`qubit` remain unboxed. Boxes use standard class rules.
