---
name: google-cpp-style
description: >
  CRITICAL — Use this skill for every request that involves writing, modifying,
  reviewing, or analyzing C++ code, even if the user does not explicitly mention
  style. This skill enforces the Google C++ Style Guide 100% strictly. Trigger
  on ANY of the following: the words "C++", "CPP", "cpp", "c++", "modern C++",
  "C++ idioms", or "C++ best practices"; mentions of any C++ file extension
  (.cc, .cpp, .cxx, .h, .hh, .hpp, .hxx, .ipp); action verbs applied to C++ code
  ("implement in C++", "write a class", "write a struct", "add a method",
  "implement this function", "modify this C++", "refactor my C++", "rewrite in
  C++", "port this to C++"); requests for idiomatic / clean / lint / review of
  C++ code; any use of std::, absl::, boost::, folly::, or Qt symbols; build
  questions about .cc/.h files; g++/clang/clang-cl/MSVC compiler questions about
  C++ source; or any mention of "header guard", "IWYU", "RAII", "smart pointer"
  in a C++ context. When in doubt, USE THIS SKILL — every byte of C++ you
  write or change should be reviewed against these rules. Do NOT skip this
  skill just because the user said "write me an X" where X happens to be C++.
---

# Google C++ Style Guide Skill

Use this skill whenever a user provides, asks about, or wants to modify C++ code. The rules below encode the **Google C++ Style Guide** 100%. Apply them strictly unless (a) the project is explicitly Windows-targeting (see "Windows Code" in the references) or (b) the user explicitly opts out of a specific rule.

If a rule appears in this file, it is mandatory. If you are uncertain about an edge case, do not guess — read the relevant file from `references/`.

## 0. Goals of the Style Guide (the underlying principles)

The rules that follow all serve the eight core principles Google applies in priority order. **When a rule is ambiguous, resolve it in favor of these principles, not by analogy to a different rule.**

1. **Style rules must pull their weight** — the value of a rule must justify the cost of every engineer remembering it. Don't add rules you can't defend.
2. **Optimize for the reader, not the writer.** Code is read far more than it is written. "Leave a trace for the reader" — when something surprising happens (ownership transfer, threading, etc.), make it obvious at the call site.
3. **Be consistent with existing code** — within a file, within a tightly-related surface, then within a project, then within Google's broader codebase. Consistency enables tooling that formats/fixes code automatically.
4. **Be consistent with the broader C++ community when appropriate** — adopt standard idioms unless they are objectively worse for our use case.
5. **Avoid surprising or dangerous constructs** — C++ has more pitfalls than most languages; some restrictions exist purely to prevent them. The bar for waiving such rules is high.
6. **Avoid constructs that our average C++ programmer would find tricky or hard to maintain.** Code ownership changes; complex bits become everyone's problem.
7. **Be mindful of our scale.** With 100M+ lines of C++ in active Google codebases, mistakes and shortcuts for one engineer become costly for many. Polluting the global namespace is one of the most expensive examples — name collisions at that scale are prohibitively hard to avoid after the fact.
8. **Concede to optimization when necessary.** Performance optimizations can be necessary and appropriate even when they conflict with the other principles of this document.

## 1. C++ version and tooling

- **C++ version target: C++20.** Do not use C++23 features. Do not use non-standard extensions. Consider portability to other environments before using new C++17 / C++20 features.
- Use **`cpplint.py`** to detect style violations. It is not perfect, but it catches a meaningful fraction of issues quickly. Every project should run it; many CI setups wire it into pre-submit.
- **Do NOT use C++20 modules.** Modules (`module`, `export`, `import`) are not yet well-supported by our build systems and tooling and are still being assessed for the ecosystem. Use header files instead.

## 2. Strict dealbreakers (NEVER / ALWAYS)

These are the most important rules. Violating any of them is an automatic fail.

- **NEVER use C++ exceptions.** No `try`, `catch`, `throw`. Use return values, `absl::Status`, or `absl::StatusOr<T>`.
- **NEVER use RTTI** (`dynamic_cast`, `typeid`) in production code. Use virtual dispatch or Visitor. RTTI is OK in unit tests.
- **NEVER use C-style casts** like `(int)x.5;`. Use `static_cast<>`, `const_cast<>`, `reinterpret_cast<>`.
- **NEVER use `using namespace foo;`** in headers or at namespace/global scope. NEVER `using namespace std;` anywhere.
- **NEVER declare/forward-declare anything in namespace `std`**. NEVER use `std::` of a name that doesn't exist (don't pretend to extend std).
- **NEVER use `std::auto_ptr`** (deprecated/removed).
- **NEVER define `operator&&`, `operator||`, operator comma (`operator,`), or unary `operator&`.**
- **NEVER use tabs.** Use **2 spaces** per indent level.
- **NEVER exceed 80 characters per line** (with documented exceptions for comments and raw string literals).
- **NEVER use Hungarian notation**, including Windows code.
- **NEVER omit `#define` guards** in headers. Always use `<PROJECT>_<PATH>_<FILE>_H_` format — slashes in the path become underscores. (`#pragma once` is **allowed as a secondary measure**, but the `#define` guard is mandatory.)
- **NEVER write a plain unscoped `enum`**. Always prefer `enum class` (`enum class Foo { kBar, kBaz };`). Scoped enum provides type safety and avoids implicit conversion to `int`.
- **NEVER** use the pre-C++11 "make a copy constructor private and don't define it" trick — use `= delete` instead.
- **NEVER use C++20 modules.** See section 1 above.
- **Use C++20 coroutines only via libraries approved by your project leads.** Writing your own coroutine machinery requires `awaitable` / promise types that are unique to each project; the rule is "avoid unless guided".

```cpp
// Example: header at path "foo/bar/baz.h" in project "myproject"
#ifndef MYPROJECT_FOO_BAR_BAZ_H_
#define MYPROJECT_FOO_BAR_BAZ_H_
// contents...
#endif  // MYPROJECT_FOO_BAR_BAZ_H_
```

## 3. Naming conventions (mandatory)

| Element | Style | Example |
|---|---|---|
| Type / class / alias | `PascalCase` | `UrlTableErrors`, `MyClass` |
| Concept | `PascalCase` (follows type names) | `Sortable`, `Iterator` |
| Variable / parameter / function parameter | `snake_case` | `table_name`, `num_entries` |
| Class data member (non-static) | `snake_case` + trailing `_` | `member_var_` |
| Struct data member | `snake_case` (no trailing `_`) | `name`, `count` |
| Constant (`constexpr`/`const`/`enum`) | `kPascalCase` | `kMaxRetries`, `kDaysInAWeek` |
| Function | `PascalCase` | `MyMethod()`, `OpenFile()` |
| Accessor / mutator | `PascalCase` is standard; accessors may also read like a variable name | `count()`, `is_ready()` |
| Namespace | `snake_case` (lowercase, digits OK, no leading `k`) | `my_project::utils` |
| Enumerator | `kEnumName` | `kOk`, `kNotFound` |
| Template type parameter | `PascalCase` (it's a type) | `T` for short, otherwise `MyType` |
| Macro (rare, must be project-prefixed) | `ALL_CAPS_WITH_UNDERSCORES` | `MYPROJECT_ROUND` |

Inline namespaces: forbidden, except for established versioning policy. Inline namespaces are not for "internal" use — use anonymous namespaces inside `.cc` files for that.

**Avoid `_` followed by a capital letter** (reserved for the implementation in any scope).

## 4. Files

- File names: lowercase, may include `_` and `-`. Extensions: `.cc` for source, `.h` for headers.
- C++-specific: don't use `foo.cc`, `foo.h` if there's a clash; use `foo.cpp` only if the project convention requires it.
- Every `.cc` file should have an associated `.h` file (exceptions: unit tests and small `.cc` files containing just `main()`).
- Self-contained headers are mandatory (unless `.inc` extension is used for an explicitly non-self-contained file).

## 5. Headers

- Every header has `#define` guards in the canonical format above.
- Use **Include What You Use (IWYU):** every `.cc` includes every header it uses directly, not transitively. Header files also `#include` what they use.
- Prefer full `#include` over forward declarations. Forward declarations are allowed only with measurable benefit, when you own both sides.
- Define functions in headers only when they are short (≤ ~10 lines). Otherwise, put them in `.cc` files.
- **Include order: strict & alphabetized.**
  1. Related header (the `.h` matching the `.cc`).
  2. C system headers (`<unistd.h>`, etc.) — alphabetical.
  3. C++ standard library (`<vector>`, etc.) — alphabetical.
  4. Other libraries (`<absl/...>`) — alphabetical.
  5. Your project's headers (`"foo/bar.h"`) — alphabetical.
  Each group separated by a single blank line. Project headers go last so project policy can shadow `<iostream>` if needed.

## 6. Scoping & namespaces

- All code lives in a namespace (after includes and forward declarations wrap the entire file).
- No `using namespace` at namespace/global scope. Specific `using foo::Bar;` is allowed inside `.cc` files.
- Use single-line nested namespaces in C++17+ (`namespace foo::bar { ... }`).
- Local variables declared and initialized in their narrowest reasonable scope.
- Static/global variables allowed **only** when trivially destructible; use `constexpr`/`constinit`; not allowed for non-trivial destructors (use `absl::NoDestructor<T>` if you must).
- `thread_local` follows the same rule: must be `constexpr`/`constinit` initialized, never dynamically initialized.

## 7. Classes

- Use `class` for active data with invariants; `struct` for passive data with no invariants (public members).
- Constructors must not call virtual methods.
- **Single-argument constructors and conversion operators MUST be `explicit`** (copy/move constructors exempt).
- Decide and state copy/move semantics clearly with `= default` or `= delete`. Follow the Rule of Three/Five. To **disable** copying or moving, write `<ClassName>(const <ClassName>&) = delete;` and the matching assignment as `= delete`. Do NOT use the legacy "private copy ctor with no definition" trick.
- Data members are **private**. Constants can be `public`.
- All `override`s must use `override` (or `final` on the override). Declaration exact-matches base.
- **Virtual methods must be `public` (or pure virtual).** Do not make virtual methods `private` or `protected` — it breaks subclassing and is not the Google way to disable calls.
- Inheritance: prefer composition. Public inheritance for "is-a". Avoid multiple implementation inheritance.
- Operator overloading: judicious. Never overload `&&`, `||`, comma, unary `&`. Prefer non-member for binary ops.

**Declaration order inside each access block:**

1. Types and typedefs.
2. Static constants.
3. Static methods.
4. Constructors / assignment / destructor.
5. Everything else (methods, then data members).

## 8. Functions

- Output via return values, never via non-const reference parameters. If output is needed as a parameter, use a `T*` (allowing `nullptr`) — never `T&`.
- Input before output in parameter order. Optional inputs: `std::optional<T>`. Optional outputs: `T*`.
- Keep functions short (≤ ~40 lines is a soft guide). One thing per function.
- Overload only when call site is obvious without reasoning about types. Document an overload set with one comment block.
- Default arguments allowed on **non-virtual** functions when defaults are consistent. Forbidden on **`virtual`** functions.
- Trailing return types (`auto f() -> int`) only when leading type is impractical: lambda expressions with `auto`-deduced return, template functions where the return type depends on parameter types. Never for ordinary functions.

## 9. Other C++ features

- **Smart pointers:** `std::unique_ptr<T>` for exclusive ownership. `std::shared_ptr<T>` only for genuine shared ownership. Borrow = raw pointer/reference.
- **Rvalue references (`&&`):** move ctor/assignment, `&&`-qualified methods consuming `*this`, perfect forwarding. Nothing else.
- **`noexcept`:** move ctor/assignment, swap, destructors where the lack of throwing is part of the contract.
- **Casting:** `static_cast<>`, `const_cast<>`, `reinterpret_cast<>`, `std::bit_cast<>`, or `absl::implicit_cast`/`absl::down_cast` where appropriate. **Never C-style casts.**
- **Preincrement (`++i`) preferred** over postfix (`i++`).
- **`const` everywhere meaningful** — parameters, locals, member functions where there's no mutation.
- **`constexpr` for true constants.** `constinit` for runtime types initialized at compile time. `consteval` for compile-time-only functions.
- **Integer types:** `int` for normal integers; `<cstdint>` types (`int32_t`, `int64_t`, etc.) for sized integers. **No `short`/`long`/`long long`** unless precisely required by external API. **No unsigned** unless semantically required (modular arithmetic, bitfields).
- **Floating-point types:** `double` for normal floats; `float` if matching an external format. Avoid `long double`.
- **Macros:** `#define` only for include guards. Otherwise avoid; use `inline` functions, `const`/`constexpr`, `enum class`. If a macro must exist, `#undef` it promptly and project-prefix its name.
- **Use `nullptr` for pointers (never `NULL` or `0`).** Use `'\0'` for null chars. Use `sizeof(varname)` over `sizeof(type)`.
- **Type deduction (`auto`):** use where it improves clarity (verbose iterator types, factory returns). Don't pad with `auto`. **For pointer types, use `auto*`** — `auto* p = factory();`, not `auto p = factory();`. Structured bindings (`auto [a, b] = pair;`) are encouraged when meaning is clear. **Avoid `auto x{val};` and `auto x = {val};`** — these deduce `std::initializer_list<T>`, not `T`. Use `auto x = val;` or `T x = val;` instead.
- **CTAD (class template argument deduction):** only when templates opt in via deduction guides.
- **Designated initializers (C++20):** allowed only when initializers follow declaration order.
- **Lambdas:** encouraged for short local logic. **Capture explicitly (no `[&]` or `[=]` in escaping lambdas)** — escaping lambdas with implicit ref captures risk dangling references.
- **Templates & metaprogramming:** keep simple. Avoid heavy SFINAE; use concepts sparingly. New concepts are rare and internal.
- **Aliases:** prefer `using Foo = ...;` over `typedef`. Aliases can be templated; typedef cannot.
- **`switch`:** always include `default:` unless all enum cases are explicit. Mark fall-through with `[[fallthrough]];` (not as the last case).

## 10. Comments

- Use `//` for most comments. Comment style is sentences — capitalization and punctuation.
- File-level copyright comment at top (recommended on first files).
- Class comments above class declaration (purpose, invariants, ownership).
- Function comments above declarations for non-trivial public functions: input, output, side effects, threading, performance.
- Variable comments above tricky data members (especially `static constexpr int kFoo;` magic numbers).
- Implementation comments to explain **why**, not what.
- `TODO(name):` for tasks (with name, ideally date). `FIXME:` is deprecated.
- Comments should be **complete, capitalized sentences** with periods. Aim for ~70–80% comment density on non-obvious code. Don't ASCII-art banner the file header.

## 11. Formatting — at a glance

(See `references/01-formatting-and-naming.md` for full detail.)

- 80 chars per line. 2-space indents. No tabs.
- All statements that have a body use braces, even single-line bodies. The single-line exception (`if (x) return;` without braces) is allowed only when the body is a single `return`, `break`, `continue`, or `goto` on the next line.
- Pointer/reference spacing: `char* c`, `const std::string& str`. The `*`/`&` is with the **type**, not the variable name.
- Brace style: opening brace on the **same line** as the control statement/declaration (`if (x) {`). Closing brace on its own line.
- Function call formatting: arguments on one line if they fit, otherwise one per line with 4-space continuation.
- Lambda body: braces on same line as the capture; multi-line body indented 2 spaces like a function.
- `return x;` without parens (no `return (x);`).
- Use `//` for comments. Doc comments for Doxygen only if the project uses Doxygen.
- One blank line between functions; two blank lines between class definition and following function.
- In a `.cc` file, every function definition must have its contents wrapped in a namespace; the namespace brace closes after the last function, with `}  // namespace foo`.

## How to apply this skill

When the user asks for C++ code (write / modify / review / explain):

1. **Before writing**, check this file and the relevant reference for the area you're touching (e.g. classes → `03-classes-and-functions.md`).
2. **Open `examples/url_table.h` and `.cc` first** if you're uncertain how a rule applies — they show every rule together, which beats reading five separate references.
3. **While writing**, apply every rule strictly. If you find yourself using a forbidden construct (exception, RTTI, C-style cast, `using namespace std`, C++20 modules), stop and rewrite.
4. **After writing**, do a final self-review against:
   - Section 0 (Goals — does this serve the reader?).
   - Sections 1–2 (dealbreakers).
   - Section 3 (naming).
   - Section 9 (modern-C++ specifics).
   - And the area-specific reference for the code shape (header / class / function / formatting).
5. **If you need to break a rule** (e.g. user explicitly said "use exceptions here"), add a brief comment justifying the break — but **only** when explicitly requested by the user. Otherwise, do not.

If the prompt is about **reviewing** existing code, do a strict audit using the references and report violations by category (dealbreaker / naming / formatting / restrictions).

If the prompt is about **explaining** a Google-style choice, the answer is in this file or its references.
