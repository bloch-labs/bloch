# DES-005: Stdlib Collections Package

## Scope
Implements ADR-007: reference-only collections that operate on boxed primitives and classes.

## High Level Flow
- Collections are plain classes; no compiler/runtime changes. Factories and methods are implemented within stdlib code.

## Compilation (Lexer/Parser/AST)
- No changes; collections use existing class/generic syntax.

## Semantic Analysis
- Existing rules apply: type args must extend `Object`; static/instance/visibility rules enforced as normal.

## Runtime Architecture
- Uses standard class runtime: fields, methods, constructors. No special runtime hooks.

## Stdlib Classes & APIs
- **Collections** (utility, static)
  - Static factories (optional): `ofList<T>(... values) -> List<T>`, `ofSet<T>(... values) -> Set<T>`, `ofPair<A,B>(A,B) -> Pair<A,B>`.
  - Non-instantiable (static class).

- **List<T extends Object>**
  - Ctors: `List()`
  - Static: `of(T v1, T v2?, ...) -> List<T>`
  - Methods: `add(T)`, `addAt(int index, T)`, `get(int index) -> T`, `set(int index, T)`, `removeAt(int index) -> T`, `contains(T) -> boolean`, `clear()`, `size() -> int`, `isEmpty() -> boolean`.
  - Semantics: singly linked backing is acceptable; index out-of-range throws runtime error.

- **Set<T extends Object>**
  - Ctors: `Set()`
  - Static: `of(T v1, ...) -> Set<T>`
  - Methods: `add(T) -> boolean` (true if inserted), `remove(T) -> boolean`, `contains(T) -> boolean`, `size() -> int`, `isEmpty() -> boolean`, `clear()`.
  - Semantics: uniqueness by `equals`; initial implementation may be linear or hashed but deterministic.

- **Map<K extends Object, V extends Object>**
  - Ctors: `Map()`
  - Static: `of(K k1, V v1, K k2, V v2, ...) -> Map<K,V>`
  - Methods: `put(K,V) -> V?` (previous or null), `get(K) -> V`, `containsKey(K) -> boolean`, `remove(K) -> V`, `size() -> int`, `isEmpty() -> boolean`, `clear()`.
  - Semantics: key equality via `equals`; linear map acceptable for v1.2.0.

- **Pair<A extends Object, B extends Object>**
  - Ctors: `Pair(A first, B second)`
  - Static: `of(A,B) -> Pair<A,B>`
  - Methods: `first() -> A`, `second() -> B`, `equals(Object) -> boolean` (value-based), `toString() -> string` like `(a,b)`.

## Testing
- Unit tests per class: creation, core ops, size/emptiness, error cases (out of range/missing key), equality behaviour.
- Integration: examples using boxed primitives and user classes (`List<Integer>`, `Set<String>`, `Map<Integer,String>`, `Pair<Boolean,Integer>`).

## Compatability
- Reference-only: type args must extend `Object`; primitives require boxing. No iterators/generators; all operations are eager and deterministic.
