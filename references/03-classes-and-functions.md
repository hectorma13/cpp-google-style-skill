# Classes, Structs, Functions (Detailed Reference)

This is the detailed companion to the Classes and Functions sections of SKILL.md. Read it when designing classes, choosing move/copy semantics, applying inheritance, overloading operators, or writing functions.

---

# Classes

## Doing Work in Constructors

- Constructors must not call virtual methods (the dynamic type is not the constructor's own type yet, and overrides see a half-initialized object).
- Constructors must not fail in ways that signal via non-Bool return values; if construction can fail, prefer a factory function returning `std::unique_ptr<T>` (with `absl::StatusOr`/`Status`-style if used) or call `Init()` after construction.
- A common technique:

```cpp
// Good — failure returns null/Status; the constructor is light.
class Server {
 public:
  static std::unique_ptr<Server> Create(const Options& opts);   // factory
  Status Init(const Options& opts);

 private:
  Server() = default;
};

// Usage:
auto server = Server::Create(opts);
if (!server.ok()) { /* handle */ }
```

## Implicit Conversions

- **Avoid implicit conversions.**
- Single-argument constructors and conversion operators **must** be `explicit`, EXCEPT for copy/move constructors and type-conversion constructors declared as `explicit` only in narrow exceptions (review cases individually).
- **EXCEPTION:** Copy and move constructors, and copy/move assignment, may be non-`explicit`. They are not user-defined conversion operations.
- A single-argument constructor that takes a `std::initializer_list` is OK and is **not** required to be `explicit` because the brace-initializer form would otherwise be too restrictive; still, if you don't need it non-explicit, mark it `explicit`.

```cpp
class MyClass {
 public:
  explicit MyClass(int var);
  ...
};
```

```cpp
void DoSomething(const MyClass& obj);

// Allowed (explicit constructor):
DoSomething(MyClass(5));          // OK
DoSomething(5);                   // Bad — implicit conversion blocked, which is what we want.

// Allowed (copy, so not explicit):
DoSomething(other_obj);
```

## Copyable and Movable Types

A class must explicitly choose:

- **Copyable + Movable** — the default `T(const T&) = default; T(T&&) = default;` — most types.
- **Move-only** — declare copy as `= delete`, allow move.
- **Not copyable, not movable** — both as `= delete`.

**To disable** copying or moving (any of copy ctor / copy assign / move ctor / move assign), write the operation as `= delete`. Do NOT use the legacy "private copy ctor with no body" trick — `= delete` is the modern, explicit, error-message-friendly form.

Although Google style usually prefers using `using`/`=default` or implicit generation, classes that benefit from explicit declarations should make them clear:

```cpp
// Copyable + Movable (default declarations):
class Copyable {
 public:
  Copyable() = default;
  Copyable(const Copyable&) = default;
  Copyable(Copyable&&) = default;
  Copyable& operator=(const Copyable&) = default;
  Copyable& operator=(Copyable&&) = default;
  ~Copyable() = default;
};

// Move-only:
class MoveOnly {
 public:
  MoveOnly() = default;
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly&&) = default;
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;
  ~MoveOnly() = default;
};

// Neither:
class NotCopyableOrMovable {
 public:
  NotCopyableOrMovable() = default;
  NotCopyableOrMovable(const NotCopyableOrMovable&) = delete;
  NotCopyableOrMovable& operator=(const NotCopyableOrMovable&) = delete;
  NotCopyableOrMovable(NotCopyableOrMovable&&) = delete;
  NotCopyableOrMovable& operator=(NotCopyableOrMovable&&) = delete;
  ~NotCopyableOrMovable() = default;
};
```

If your class has a destructor, you must decide whether you also need to define/copy/move-assign (the Rule of Three/Five). Either delete them explicitly or define them.

## Structs vs. Classes

- Use `struct` for passive data structures with **public data members** and no (or minimal) invariants.
- Use `class` for **active data structures** with private data, invariants, methods, etc.
- An aggregate type with public member variables and no methods: `struct`.
- If you find yourself adding invariants to a `struct`, convert to `class`.

## Structs vs. Pairs and Tuples

- Pairs and tuples are most appropriate in generic algorithms where the meaning is not specific.
- Prefer a **named `struct`** (with or without an associated class) when the fields have meaningful names.

```cpp
// Prefer this:
struct PageInfo { std::string url; int64_t size_bytes; Date fetched_at; };

// Over this:
std::tuple<std::string, int64_t, Date>;
```

## Inheritance

- Composition is almost always a better choice than inheritance. Use it first.
- When inheritance is appropriate, use **public** inheritance (matches "is-a").
- Data members should be `private`. The base class can grant access via `protected` members; however, **make data members `private`** in derived classes too — derived doesn't need access to internal data of base, only behavior.
- Implementations that need access to internals of multiple concrete classes in the same inheritance hierarchy **should not** be implemented as virtual methods. The result is "fat" base classes.
- Limit the use of multiple inheritance to only when at most one of the bases has data members; document the reasons.
- **Virtual methods must be `public`** — except when they are pure virtual (which lets the base stay abstract). Never make a virtual method `private` or `protected` "to discourage callers from using it" — that breaks subclasses in subtle ways and isn't how Google disables access.
- For `virtual` methods, use `override` (or `final`) on every override.
- Type signatures of `override`s match exactly the base (qualifiers, ref-qualifiers, defaulted args, return types).

```cpp
class Derived : public Base {
 public:
  void Foo() override;
  void Bar() const override;
  ...
};
```

## Operator Overloading

> **Heads up before defining an enum:** always use `enum class` (scoped), not plain `enum` (unscoped). Plain `enum` leaks its enumerators into the enclosing namespace and silently converts to int. `enum class MyEnum { kA, kB };` is the correct form; enumerators follow the `kEnumName` naming rule.

```cpp
enum class UrlTableError {
  kOk = 0,
  kNotFound,
  kNetworkError,
};
```

- Use **judiciously**. Don't define overloads that obscure the language semantics.
- **Never** overload:
  - `&&`, `||`, comma (`,`), unary `&`.
  - Any operator that doesn't have a conventional meaning.
- Do not overload `operator""` — too easy to shadow standard-library UDLs.
- If you define `operator==`, also define `<=>` (C++20) or at least ensure it's logically consistent with `operator!=` and `/`.
- Prefer **non-member** operators for binary operators when they don't need access to private state.
- If your class is used as a value type (e.g. `Money`, `Complex`), defining a few operators is appropriate.
- Document operators in the same place as the class — comments should still apply.

## Access Control

- Data members are `private` (with `static constexpr` constants allowed at `public` scope).
- Provide accessors and mutators as needed: do not expose internal state through `protected`. (`protected` data leads to base-class-spiral designs where invariants are hard to maintain.)
- Const accessors return `const T&` if the type is large; return by value if the type is small (`int`, `bool`).

## Declaration Order

Inside each access block:

1. **Types and typedefs** (`using`, `enum class`, nested classes).
2. **Static constants.**
3. **Static methods.**
4. **Constructors / assignment operators / destructor.**
5. **Methods (other than static).**
6. **Data members.**

Constructor and destructor order is usually source-relevant (mostly top-of-class), but within types this is the canonical Google order.

---

# Functions

## Inputs and Outputs

- Functions should be **parameter lists**, not "out parameters."
- Use return values. If multiple values are needed, return a `struct`/`std::tuple`/`absl::StatusOr` (when allowed).
- **Avoid non-const reference parameters** — output parameters.
  - If you must, prefer a **pointer** (signaling "may be null" or "is an out-parameter"). `T* out_value` makes intent obvious.
- Optional inputs:
  - Pass-by-value `std::optional<T>` for "may not be present" semantics.
- Optional outputs:
  - `T*` (with `nullptr` meaning "not requested").
- Ordering: input parameters first, then output parameters. Don't sprinkle output params randomly.

```cpp
// Good
void Compute(const Input& in, Result* out);

// Less ideal:
void Compute(const Input& in, Result& out);  // output is non-const ref — harder to read.

// Best:
Result Compute(const Input& in);             // return value if possible.
```

## Write Short Functions

- Prefer **small, focused** functions. Rule of thumb: if exceeding ~40 lines, look for places to break it up.
- A function should do one thing: clearly named, clearly bounded, clearly testable.
- Long functions harm readability, testability, and refactorability.

## Function Overloading

- Use overloading only when no reasoning about parameter matching is required at the call site.
- Example: `ABS` is overloaded for integers and doubles (`std::abs` does this). Call site is obvious.
- **Avoid overloading** where:
  - It's not clear which overload will be called.
  - The overloads have the same number of parameters and similar types.
- Document the overload set with a single "umbrella" comment near the class or namespace declaration.

```cpp
// Good — int-overloaded functions that everyone can call.
std::string ToString(int i);
std::string ToString(double d);
```

## Default Arguments

- Allowed on **non-virtual** functions when the default is always the same.
- Allowed on **virtual** functions are **discouraged**; if default behavior differs across overrides, callers can't easily know what they'll get.
- **Forbidden on `virtual`** — virtual functions should be overridden consistently.
- **Prefer overloading** when default args would surprise the reader.

```cpp
// Allowed:
class MyClass {
 public:
  void DoSomething(int n = 10);
};

// Bad:
class Base {
 public:
  virtual void Foo(int n = 10);   // Bad — default differs across overrides will mislead callers.
};
```

## Trailing Return Type Syntax

Use trailing return types **only** when the leading return type would be impractical. Two common cases:

- Lambda expressions where the return type must be deduced.
- Template functions where the return type depends on the parameter types (e.g. `decltype(a + b)` makes the relationship with the operands explicit).

```cpp
// Template whose return type depends on the parameter types.
template <typename T, typename U>
auto Add(T a, U b) -> decltype(a + b) { return a + b; }
```

For ordinary functions, **always prefer the leading return type** (`int Foo() { ... }`, not `auto Foo() -> int { ... }`). The trailing form forces the reader to scan past the parameter list to learn what the function returns.

---

# Google-Specific Magic

The style consistent with Google codebases typically references:

- `absl::Status` and `absl::StatusOr<T>` (in codebases that use these) for fallback semantics instead of exceptions.
- `cpplint.py` for static style checks.
- Project headers and project conventions take precedence over the wider guide if there is a conflict (e.g. an internal intra-classes naming pattern).

If you are writing code for an internal Google-style project, follow those project conventions when they conflict with the general guide. Otherwise, follow the guide strictly.
