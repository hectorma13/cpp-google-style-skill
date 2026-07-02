# Restrictions & Modern C++ Features (Detailed Reference)

This is the detailed companion to the "Other C++ Features" section of SKILL.md. Read when you're working with smart pointers, casts, lambdas, templates, integer types, or other modern C++ features — and when you need to know what's forbidden.

---

# Ownership and Smart Pointers

- **Prefer transferring ownership** — `std::unique_ptr<T>` for exclusive ownership, with `std::move()` to hand it off.
- **Avoid `std::shared_ptr<T>` unless you genuinely need shared ownership.** Most "shared" use cases are really exclusive-ownership with a borrow.
- **`std::auto_ptr` is FORBIDDEN** — deprecated and removed.
- Always pass `std::unique_ptr` to functions that take ownership **by value**.
- Never assign the same raw pointer to two `unique_ptr`s.

```cpp
std::unique_ptr<Foo> MakeFoo();          // factory
void UseFoo(std::unique_ptr<Foo> f);     // takes ownership by value

auto foo = MakeFoo();
UseFoo(std::move(foo));                  // pass
```

Decide smart-pointer vs. raw pointer at the type level:

- **Owner** of the object's lifetime: `std::unique_ptr<T>` (use `std::make_unique` if available; otherwise `new`).
- **Borrower** (not an owner): raw pointer or reference (prefer `T*` if nullable, `T&` if not).
- If a non-owner needs to express "I might outlive this — keep it alive for me," use `absl::WeakPtr` or similar (in codebases that use it).

Place unique ownership at the lowest practical level — a long-lived object holds a `unique_ptr`; transient borrows are raw pointers.

# Rvalue References

- Use `&&` **only**:
  1. With **move constructors** and **move assignment operators** (and **forwarding constructors**).
  2. To define **overload sets** (e.g. `&&`-qualified "rvalue-only" methods).
  3. For **perfect forwarding** in `std::forward` chains (wrap-around via `T&&` + `std::forward<T>`).
- Don't write `T&& x` as a casual parameter type just for documentation; the `&&` has real meaning — use it correctly.
- A method that's `&&`-qualified (e.g. `void Foo() &&`) is allowed; it conveys "consumes `*this`."

# Friends

- Permitted within reason: `friend class MyClass;` is fine when there's a clear tight coupling.
- The use of `friend` in tests is acceptable when the production class must not expose its internals.
- Generally **define friend functions inline** in the same header as the class. Don't separate `friend` declarations from the class into other source files.

# Exceptions

- **C++ exceptions are prohibited in this style guide.** Do NOT use `try`, `catch`, `throw`.
- Rationale: control-flow complexity, interaction with cleanup, history of Google's C++ on certain platforms, and integration with code that doesn't yet use exceptions.
- Fallback semantics:
  - Return `bool`/an error code (for simple functions).
  - Use `absl::Status` / `absl::StatusOr<T>` (in projects adopting Abseil) or similar value-or-error return types.
  - Use early returns and explicit error checks at each call site.

```cpp
// Bad — exception-based error handling.
bool OpenFile(const std::string& path) {
  try {
    f_.open(path);
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

// Good — explicit error reporting.
absl::Status OpenFile(const std::string& path) {
  f_.open(path);
  if (!f_.is_open()) return absl::ErrnoToStatus(errno, "open failed");
  return absl::OkStatus();
}
```

The style guide explicitly permits the **use of `absl::Cleanup`** or other RAII patterns — but never relies on catch-blocks.

# `noexcept`

- Apply `noexcept` to functions where omitting exceptions is part of the contract: move constructors, move assignment, swap, destructors.
- For functions that "should" not throw but where you're not sure, use `noexcept(false)` or no annotation rather than over-claiming.
- Don't write overly complex `noexcept(...)` clauses. Simple cases only.

```cpp
class Buffer {
 public:
  Buffer(Buffer&&) noexcept = default;
  Buffer& operator=(Buffer&&) noexcept = default;
  void swap(Buffer& other) noexcept {
    std::swap(data_, other.data_);
  }
};
```

# Run-Time Type Information (RTTI)

- **RTTI is forbidden** in production code.
- Avoid `dynamic_cast`, `typeid`, `std::type_info`.
- Use **virtual methods** to dispatch on type, or use **double-dispatch** / the **Visitor** pattern.
- RTTI is **allowed in test code**, **only** when there's a clear test-isolation reason.

```cpp
// Bad — RTTI-based switch.
if (auto* p = dynamic_cast<Derived*>(base.get())) {
  p->DoSomething();
}

// Good — virtual dispatch.
base->Foo();
```

# Casting

- **NEVER** use C-style casts: `(int)x`.
- Use C++ casts:
  - `static_cast<T>(x)` — explicit type conversion when the conversion is well-defined.
  - `const_cast<T>(x)` — only to strip `const` (rare; indicate real intent with a comment).
  - `reinterpret_cast<T>(x)` — pointer/integer conversion; never convert between unrelated types casually. Document the use.
  - `absl::implicit_cast<T>(x)` — performs a chain of implicit conversions, only when intentional.
  - `absl::down_cast<T>(x)` — for downcasting after an `absl::up_cast` chain in performance-sensitive code: explicit and safe.
  - `std::bit_cast<T>(x)` — bit-level reinterpretation (C++20+).
- For conversions to **integer**: the source is `float`/`double`, the destination is an integer — `static_cast<int>(x)` is correct.
- For **brace initialization**: `{int x = 3.5;}` is forbidden — narrowing conversion; `int x{3.5};` is also a compile error. Use `int x = static_cast<int>(3.5);`.

# Streams

- Use only where they make sense: debug logging, test diagnostics, simple formatting in non-user-facing tools.
- **Don't** use streams for:
  - Heavy user-facing I/O.
  - Internationalization (`<<` doesn't help with localization).
  - Anywhere you need fine-grained formatting precision.
- **Avoid stateful / persistent stream usage** — calling `std::setprecision` etc. inside production code is fragile.

```cpp
// OK — debug debug print.
LOG(INFO) << "Loaded " << items.size() << " items.";

// Bad — user-facing text via stream.
std::cout << "Hello, " << user.name() << "!\n";  // don't.
```

# Preincrement and Predecrement

- Use **prefix** (`++i`) instead of **postfix** (`i++`).
- Reason: postfix creates a copy of the original; for non-trivial iterators this is wasted work.
- The only place to use `i++` is when the postfix semantics are explicit (e.g. "I want the value before increment").

# Use of `const`

- Use `const` everywhere it would meaningfully document intent:
  - `const int kGlobalLimit = 5;`
  - Constants in methods that don't mutate state: `int Count() const;`
  - Pointers/references that don't change: `void Bar(const Foo& f);`
- Use `const` on local variables where it makes the intent clear, but don't pepper `const` on random temporaries.
- `mutable` is allowed in narrow cases (e.g. caches, mutexes), with comment.

# `constexpr`, `constinit`, and `consteval`

- Use `constexpr` for **true compile-time** constants and for functions that can be evaluated at compile time.
- `constexpr` is the Google style for everything that was a `#define` macro in C.
- Use `constinit` (C++20) for variables that should be statically initialized to a constant expression but whose type is non-literal.
- Use `consteval` (C++20) for functions that **must** be evaluated at compile time.
- Don't contort a function just to make it `constexpr` if the value can be `runtime` cleanly.

```cpp
constexpr int kMax = 10;
constinit int g_counter = 0;     // initialized to a constant expression.
```

# Integer Types

- Use `int` for "ordinary" integers unless a specific size is required.
- For specific sizes use `<cstdint>` (`int16_t`, `int32_t`, `int64_t`).
- Use `<cstdint>` long enough: `uint16_t`, `int64_t`, etc.
- Use `<cstddef>` (`size_t`, etc.) for sizes / counts.
- **Avoid** `unsigned int` if you only need to express "non-negative" — use `int` unless semantics are modular/integer-bitfield.
- **NEVER** use `short`, `long`, `long long` if a sized type or `int` will do.

# Floating-point Types

- Use `double` for "ordinary" floats.
- Use `float` only when memory is tight or when you're matching an external format (e.g. graphics APIs).
- `long double` is **discouraged** — it has different precision across platforms.

# Architecture Portability

- Portable code only.
- Use `absl::StrCat`, `std::to_string()`, `snprintf` for numeric formatting — never trust endianness or `long long` widths.
- Use `absl::string_view` or `std::string_view` instead of raw memory buffers.
- Use `uintptr_t` (or `intptr_t`) for arbitrary pointer→integer conversions; do not assume `void*` round-trips to `long`.
- For 64-bit integers use `int64_t`/`uint64_t`/`absl::int128`.
- For fixed-size structs crossing ABI boundaries, use the explicit-width types above.

# Preprocessor Macros

- Avoid macros — prefer `inline` functions, `enum class`, `constexpr` variables, `typedef`.
- If you must use a macro:
  - Don't use it for control flow (avoid `if (DEBUG) ...`).
  - Beware of double evaluation: `#define SQUARE(x) ((x) * (x))` evaluates `x` twice.
  - Avoid `##` for token pasting — it surprises people.
  - `#undef` after use; use the lowest scope possible.
  - The macro name should be all-caps, project-prefixed.

```cpp
// Avoid:
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Prefer:
inline int Max(int a, int b) { return a > b ? a : b; }
```

`#define` for include guards is the **only** allowed common usage.

# Constants

- Use `nullptr` for pointers. **Never** `NULL` or `0`.
- Use `'\0'` for null characters. **Never** `0` for char.
- Use `sizeof(varname)` for size of a variable's type; **avoid `sizeof(type)`** where possible.

# Type Deduction (`auto`)

- Use `auto` when it improves **clarity and safety**, not just to avoid typing.
- **For pointer types, write `auto*` instead of `auto`.** This makes the type explicit at the call site and prevents "I forgot `Foo*`" confusion: `auto* p = factory();` is preferred over `auto p = factory();`. **Exception:** when the factory name makes the smart-pointer type obvious (e.g. `auto p = std::make_unique<Foo>();`, `auto cv = std::make_shared<Config>(...);`), plain `auto` is fine — adding `*` would be misleading because the result is a smart pointer, not a raw pointer.
- **Avoid `auto x{val};` and `auto x = {val};`** — these deduce `std::initializer_list<T>`, not `T`. In C++14 `auto x{1};` is `std::initializer_list<int>`; in C++17 `auto x{1};` was changed to `int` but `auto x = {1};` is still `std::initializer_list<int>`. Prefer either `auto x = val;` (copy-init) or write the explicit type.
- Allowed:
  - **Iterator types:** `for (auto it = v.begin(); ...)` is OK when the type isVerbose.
  - **New types via factory functions:** `auto opt = ParseOpts("foo");`
  - **Lambda captures and results of generic helpers.**
- Avoid `auto` when the resulting type is unclear to the reader:
  - `auto result = SomeFunction();` — reader has to look up `SomeFunction`'s return type.

  Better: `int result = SomeFunction();` if `int` is what you mean.

- Return type deduction (`auto` return type) is allowed but **discouraged** — readers prefer a declared return type. Use trailing return types for templates when needed.

- **Structured bindings (`auto [a, b] = foo;`)** are encouraged in C++17+ when the meaning is clear:

```cpp
auto [key, value] = *map.find(id);
```

# Class Template Argument Deduction (CTAD)

- Allowed only for templates that have **explicit deduction guides** (or that the standard opted in).
- If you use deduction guides, write them with care — they're rarely needed.
- Prefer writing out the type for templates that don't offer guides.

# Designated Initializers (C++20)

- Designated initializers are permitted only in their C++20 form.
- Initializers in a designated initializer must appear **in the same order** as the declaration of the fields.

```cpp
struct Point { int x; int y; };
Point p{.x = 1, .y = 2};        // OK — default initializer order
Point p{.y = 2, .x = 1};        // Bad — reverses declaration order
```

# Lambda Expressions

- Lambdas are encouraged for concise local logic.
- **Capture explicitly** when the lambda might **escape the current scope** (e.g. passed to a background thread, stored beyond the call frame).
  - Use `[&]` for by-reference capture when the lambda is used only within the enclosing scope.
  - Use `[=]` for by-value capture.
  - **Avoid `[&]` for escaping lambdas** — risk of dangling references.
- **Avoid default captures entirely** when the lifetime is unclear.

```cpp
// OK — short-lived lambda.
std::sort(v.begin(), v.end(), [a, b](int x, int y) { return x < y; });

// Bad — escaping capture with [=].
std::thread([&]() { do_work(some_local); }).detach();   // Some_local may be gone by the time the thread runs.
```

- Use a lambda **statement form** when appropriate:
  `auto cmp = [](int a, int b) { return a < b; };`

# Template Metaprogramming

- Avoid complicated template metaprogramming.
- It is hard to debug, hard to refactor, hides errors, and slows compile time.
- If you do need it, keep it minimal and **internal** to a library — user-facing headers should be readable.
- Prefer concepts and `if constexpr` to SFINAE.

# Concepts and Constraints (C++20)

- Use concepts **sparingly** — only in cases where you would have used templates before.
- Prefer `template <typename T> requires Sortable<T>` to `template <Sortable T>` when adding concepts to existing templates.
- Don't rename existing function signatures just to add concepts.
- New concepts should be rare and internal to libraries.
- Concept names follow type names (`PascalCase`) — see `references/01-formatting-and-naming.md` § Concept Names.

# Coroutines (C++20)

- **Only use C++20 coroutines via libraries that have been approved by your project leads.** Don't write your own promise / awaitable machinery.
- Coroutines are mostly used for asynchronous I/O and generators. Use sparingly.
- Avoid letting coroutines leak into the rest of the codebase except for a few well-defined places.
- C++ does not ship a standard coroutine implementation (unlike Kotlin, Rust, TypeScript). Each project must define its own awaitable / promise type, which determines parameter types, execution semantics, and hook points. This means every coroutine surface is essentially custom — the rule above reflects that reality.

# cpplint

- Use **`cpplint.py`** to detect style violations. It is not perfect, but it catches a meaningful fraction of issues quickly enough that the cost is paid back many times over.
- For large projects it may be too aggressive in some checks (e.g. include order, very long lines) — in that case, disable specific checks per-project rather than abandoning cpplint.
- Some projects have legitimate reasons to waive cpplint checks (Windows-specific headers, generated code, third-party vendored code). For those, document the waiver.
- Run cpplint in CI / pre-submit whenever feasible. A waived line still costs reader attention if it's later wrong.

# C++20 Modules

- **Do not use C++20 Modules.** (`module`, `export`, `import`)
- Modules are a major shift in how C++ is written and compiled. Google's tooling, build systems, and idioms have not yet converged on supported, vetted practices for modules. Stay on header-file machinery until guidance changes.
- If you inherit a header that currently uses `#include` and you maintain both sides, **do not** migrate it to a module unilaterally.

# Switch Statements

- **Always** have a `default:` case **unless** switching on an enum and you handle all enumerators explicitly.
- Mark fall-throughs with `[[fallthrough]];` unless they are consecutive case labels.
- Empty cases are OK; multiple cases with the same body are also OK.

```cpp
switch (x) {
  case 1:
  case 2:
    HandleBoth();
    break;
  case 3:
    HandleThree();
    [[fallthrough]];                  // explicit
  case 4:
    HandleFour();
    break;
  default:
    HandleDefault();
}
```

# Inclusive Language

- Use neutral, professional, inclusive language in comments, identifiers, error messages, and documentation.
- Avoid terms that are unwelcoming or which have acquired problematic connotations.

# `<windows.h>` Macros Conflict (NOMINMAX / WIN32_LEAN_AND_MEAN)

- `<windows.h>` defines `min` and `max` as **preprocessor macros**, which breaks `std::min`, `std::max`, and any code using `min(...)` / `max(...)` as identifiers.
- Add `#define NOMINMAX` **before** any `#include <windows.h>` to suppress the `min`/`max` macros. Add `#define WIN32_LEAN_AND_MEAN` to shrink the included surface and drop several other notorious macros.
- Whenever you write a Windows-targeted `#include <windows.h>`, do it once at the top of the source/header with the macro defined first:

```cpp
#define NOMINMAX            // suppress min/max macros
#define WIN32_LEAN_AND_MEAN  // exclude rarely-used APIs
#include <windows.h>
```

This is one of the few macro workarounds Google endorses; do it correctly. Likewise `#include <windows.h>` is the only place where Google accepts using these cppdefs.

# Disallowed Standard Library Features

Don't use:

- `std::iostream` for regular I/O — use `absl::StrCat` / `std::string` / explicit logging.
- `std::chrono::high_resolution_clock` — use `absl::Now()` when needed.
- `std::random_shuffle` — use `std::shuffle` (C++17).
- `std::auto_ptr` — removed in C++17 but reject it.

# Third-party Libraries

- Use libraries that match the project's already approved list — in a Google codebase, that means `absl/*`, `protobuf`, etc.
- For new dependencies, prefer libraries that are already in use elsewhere.

# Nonstandard Extensions

- **Don't use nonstandard extensions** unless you've checked that they're portable across all platforms the project targets.

---

# Aliases

- Use `using` aliases (C++11+) — type aliases are part of modern C++.
- `using Vec = std::vector<int>;` is fine.

```cpp
// Use `using` over `typedef`. Aliases can be templated; typedef cannot.
template <typename T>
using Vec = std::vector<T>;
```
