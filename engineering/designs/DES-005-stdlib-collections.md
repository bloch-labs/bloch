# DES-005: Stdlib Collections Package

## Scope
* ADR-007

## High Level Flow
- Collections are plain classes; no compiler/runtime changes. Factories and methods are implemented within stdlib code.

## Compilation (Lexer/Parser/AST)
- No compilation changes.

## Semantic Analysis
- No semantic analysis changes.

## Runtime Architecture
- No runtime changes.

## Stdlib Classes & APIs

- **List<T extends Object>**
  - `static of(T v1, T v2?, ...) -> List<T>`
  - `add(T) -> void`
  - `get(int index) -> T`
  - `contains(T) -> boolean`
  - `clear() -> void`
  - `size() -> int`
  - `isEmpty() -> boolean`.
  - Semantics: singly linked backing is acceptable; index out-of-range throws runtime error.

- **Set<T extends Object>**
  - `static of(T v1, ...) -> Set<T>`
  - `add(T) -> boolean` (true if inserted)
  - `remove(T) -> boolean`
  - `contains(T) -> boolean`
  - `size() -> int`
  - `isEmpty() -> boolean`
  - `clear() -> void`.
  - Semantics: uniqueness by `equals`; initial implementation may be linear or hashed but deterministic.

- **Map<K extends Object, V extends Object>**
  - `static of(K k1, V v1, K k2, V v2, ...) -> Map<K,V>`
  - `put(K,V) -> void`
  - `get(K) -> V`
  - `containsKey(K) -> boolean`
  - `remove(K) -> V`
  - `size() -> int`
  - `isEmpty() -> boolean`
  - `clear() -> void`.
  - Semantics: key equality via `equals`; linear map acceptable for v1.2.0.

- **Pair<A extends Object, B extends Object>**
  - `static of(A,B) -> Pair<A,B>`
  - `first() -> A`
  - `second() -> B`
  - `equals(Object) -> boolean` (value-based)
  - `toString() -> string` in the form `(a,b)`.

## Testing
- Unit tests per class: creation, core ops, size/emptiness, error cases (out of range/missing key), equality behaviour.
- Integration: examples using boxed primitives and user classes (`List<Integer>`, `Set<String>`, `Map<Integer,String>`, `Pair<Boolean,Integer>`).

## Compatability
- Reference-only: type args must extend `Object`; primitives require boxing. No iterators/generators; all operations are eager and deterministic.
