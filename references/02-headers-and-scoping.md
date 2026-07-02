# Headers & Scoping (Detailed Reference)

This is the detailed companion to the Header Files and Scoping sections of SKILL.md. Read it when you need exact `#include` ordering, forward-declaration guidance, namespace mechanics, or `static`/`thread_local` rules.

---

# Header Files

## Self-contained Headers

- A header file must be **self-contained** — it can be compiled on its own.
- It must include all of its dependencies. It must include its own `// Forward declarations` cleanly and end with no missing-file errors when consumed directly.
- Files designed to be included in unusual places (e.g. only after particular templates or macros) end in `.inc` instead of `.h`. They are not subject to the self-contained-header requirement.
- The exception file extension `.inc` is rare; in most cases, fix the header to be self-contained instead.

## The `#define` Guard

- All header files **must** use `#define` guards (not `#pragma once`, in general — Windows code may use `#pragma once` in addition, but `#define` guards are still required).
- Format: `<PROJECT>_<PATH>_<FILE>_H_`.
- Use underscores between words. Trailing `_H_` is the convention.
- Do not include any other content (no comments) inside the guard range.

```cpp
#ifndef FOO_BAR_BAZ_H_
#define FOO_BAR_BAZ_H_

// contents

#endif  // FOO_BAR_BAZ_H_
```

`#endif` line is annotated with the macro name so big files remain readable.

## Include What You Use (IWYU)

- Every `.cc` file must `#include` every header that defines a symbol it uses — directly, not indirectly.
- "Companion" headers (defined together) are not an excuse to omit the direct include.
- Header files should `#include` what they use too — they should already be self-contained, but a header that uses another symbol must include its header.

Example:

```cpp
// In foo.cc
#include "foo.h"          // Has bar.h because foo.h uses bar::Bar; foo.cc still includes bar.h.
#include "bar.h"          // <-- Required. Don't rely on transitive inclusion from foo.h.
```

## Forward Declarations

- Avoid when possible. Use `#includes` instead.
- Allowed cases: a clear, necessary performance benefit (e.g. a struct member used by reference in a hot header), or where you control both definitions and forward declarations are unambiguously correct.
- Always prefer `#include` unless there is a real reason. Forward declarations are fragile — types change, ownership may move, `const` may be added, etc.

```cpp
// Allowed: foo's struct uses Bar only by reference in the header.
class Bar;        // forward declared in foo.h
struct Foo {
  Bar* bar_;
};
```

Forward declarations are **not allowed** when:
- The struct is the template parameter of another type you are declaring.
- The struct is a member (not pointer/reference) of the class.
- The struct needs `sizeof` info.

## Defining Functions in Headers

- Only define functions (including inline functions) in headers if they are **short** (rule of thumb: 10 lines or fewer). Otherwise, put them in `.cc` files.
- Long definitions in headers slow compile time and are not actually inline.
- Keep public headers readable: if you need complex templates, hide them behind an opaque type or interface.

## Names and Order of Includes

The order is **strict**:

1. **Related header** (the matching `.h` for the `.cc` you are writing, or the header itself for a header template — `template <typename T> class Foo;` etc. — wait, that's not an include).
   For a `.cc` file: first include its matching `.h` (e.g. `foo.cc` first includes `foo.h`).
2. **C system headers** (e.g. `<unistd.h>`, `<stdlib.h>`).
3. **C++ standard library headers** (e.g. `<vector>`, `<string>`).
4. **Other libraries' headers** (e.g. `<absl/strings/str_cat.h>`).
5. **Your project's headers** (e.g. `"foo/bar.h"`).

Each group is separated by a single blank line. Within each group, sort **alphabetically**. No blank lines within a group.

```cpp
// foo.cc — canonical include order
#include "foo.h"            // 1. Related header (matches this .cc).

#include <sys/types.h>      // 2. C system headers, alphabetical.
#include <unistd.h>

#include <string>           // 3. C++ standard library, alphabetical.
#include <vector>

#include "absl/strings/str_cat.h"   // 4. Other libraries, alphabetical.
#include "absl/time/time.h"

#include "bar.h"            // 5. Project headers, alphabetical.
#include "baz.h"
```

Each group is separated by a **single blank line**. Within each group, sort **alphabetically**.

Project headers come **last** so they can resolve conflicts in favor of project policy (e.g. a project may shadow `<iostream>`).

You may also use `condition` or `compile_error` blocks around includes, but only if unavoidable (conditional compilation, platform-specific code).

---

# Scoping

## Namespaces

- **All code lives in a namespace.** Exceptions: `#define` macros, common-abbreviations header `math.h`, etc. — but most code follows this rule.
- After includes and forward declarations, wrap the entire source file in a namespace.
- **Forbidden:**
  - `using namespace foo;` at namespace or global scope (especially `using namespace std;`).
  - Inline namespaces — except for established cross-versioning policies.
  - Declaring or forward-declaring anything in namespace `std`.
  - Namespace aliases in header files at global scope unless they are restricted to internal-only contexts.
- **Allowed:**
  - `using namespace foo::bar;` **inside `.cc` files** is borderline discouraged but accepted in narrow cases.
  - `using foo::Bar;` inside namespaces for specific names — preferred over `using namespace`.
  - Namespace aliases **inside namespaces** in `.cc` files (e.g. `namespace fb = foo::bar;` for short aliases).
  - Single-line nested namespace declaration (`namespace foo::bar { ... }`).

```cpp
// Good — entire file inside a namespace.
#include "foo.h"

namespace foo {

// ... contents at base indent (no extra indent inside the namespace).

}  // namespace foo
```

```cpp
// C++17 nested names — one line.
namespace foo::bar {
class Baz {};
}  // namespace foo::bar
```

```cpp
// Bad — using-directive at namespace scope.
namespace baz {
using namespace foo;   // Bad — name pollution.
}
```

## Internal Linkage

- For definitions that should **not** be accessible outside the `.cc` file:
  - In a `.cc` file, use either an **unnamed namespace** or a **`static`** declaration.
  - Use either; both are accepted. Unnamed namespaces give unique names automatically.
- **Do not** put unnamed namespaces or `static` declarations in `.h` files.

```cpp
// foo.cc:
namespace foo {

namespace {
constexpr int kInternalMagic = 42;
}  // namespace

void PublicFunction() {
  // uses kInternalMagic
}

}  // namespace foo
```

Do not put `static` at namespace scope in `.cc` files in a weird way; the standard idiom is `namespace { ... }`.

## Nonmember, Static Member, and Global Functions

- Prefer placing nonmember functions inside a **namespace**, not in the global scope.
- Avoid creating classes just to group static member functions — classes are for state and invariants. Static functions go in a namespace or in a free `namespace { ... }` block.
- If a function is closely tied to a class but does not need access to private state, make it a free function in the same namespace as the class.

## Local Variables

- Declare variables in the **narrowest reasonable scope**.
- Initialize at the point of declaration. C++ allows declaration anywhere; don't defer to the top of the function.
- The first time a value is read, declare it. Prefer:

```cpp
// Good
for (const auto& item : items) {
  Process(item);
}

// Avoid
size_t i;
for (i = 0; i < items.size(); i++) {
  Process(items[i]);
}
```

- For loops and `if` (C++17) — declare the loop variable in the `for` and the temporary in the `if`:

```cpp
if (const auto* p = FindThing()) {
  Use(*p);
}
```

## Static and Global Variables

- **Forbidden** unless the variable is **trivially destructible** (has a trivial destructor or is `constexpr`/`constinit`-initialized).
- Trivially destructible means: destructor does nothing or there is no destructor (build it via `constexpr`, integer constants, plain POD types).
- For dynamically-initialized globals, the initialization order is not guaranteed across translation units; this creates brittle "static initialization order fiasco" bugs.
- **Use:**
  - `constexpr` for compile-time constants.
  - `constinit` for variables initialized by a constant expression (C++20+).
  - `absl::NoDestructor<T>` (or a do-nothing destructor wrapper) when you really need a global with a non-trivial destructor.

```cpp
// Good — trivial integer constant.
constexpr int kMaxRetries = 3;

// Good — initialized at compile time.
constinit int g_counter = 0;
```

`thread_local` is allowed but treated like a global: must be initialized at compile time, must use `constinit`, and the same destruction-order caveats apply.

## Sound C++

- Whenever you write a static/global variable, ask:
  1. Is it trivially destructible? If no, **redesign**.
  2. Is its initialization order well-defined?
  3. Is it really needed at file scope? Often it should be a function-scope local, or a static function-local variable.
