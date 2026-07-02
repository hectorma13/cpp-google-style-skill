# Formatting & Naming (Detailed Reference)

This is the detailed companion to the Formatting and Naming sections of SKILL.md. Read it when you need exact rules on whitespace, brace placement, line wrapping, naming edge cases, or the Good/Bad examples behind a rule.

---

## Line Length

- Maximum line length is **80 characters**.
- Exceptions: comment lines can be longer if they contain long URLs or quoted strings that cannot be wrapped, or raw string literals.
- `#include` statements with long paths are an allowed exception; if it does not fit, break on slashes and align subsequent lines.

```cpp
// OK — include line is one of the documented exceptions.
#include "long/project/path/SomeReallyLongHeaderName.h"
```

## Non-ASCII and Special Characters

- **Use UTF-8** for non-ASCII characters.
- Hexadecimal escapes (`\xNN`) and Unicode escapes (`\uNNNN`) are not encouraged but allowed.
- Avoid rare encodings (UTF-16, Latin-1, etc.).

## Spaces vs. Tabs

- Use **2 spaces** per indent. **NEVER use tabs.**
- Do not rely on tab-stop expansion; align with spaces.

---

## Function Declarations and Definitions

- Return type and function name appear on the same line unless wrapping.
- Parameters align wrapped on the first parameter (or the open paren if no name).
- Trailing close-paren stays on its own line.

```cpp
ReturnType ClassName::FunctionName(Type par1, Type par2) {
  DoSomething();
  ...
}

ReturnType LongFunctionName(const std::string& par1,
                           const std::vector<int>& par2) {
  ...
}
```

- Wrapped parameters have **4-space indent from the start of the line**.
- Function body is opened with `{` at the end of the declaration line (no newline before).

## Lambda Expressions

- Short lambdas: `std::sort(v.begin(), v.end(), [](int a, int b) { return a < b; });`
- If the body spans multiple lines, write it like a normal function — `{` on the same line, body indented 2 spaces, `}` aligned with the open.
- For empty capture (`[]`), use the form `[&] { ... }` when capturing by reference is intended — empty captures are unusual and should be intentional.

## Floating-point Literals

- Always include the decimal point, even when not required: `1.0` not `1`, `0.5f` not `.5f`.
- Use a digit prefix when needed (e.g. `0.0f`, `1.0`, `0.5`).
- **NEVER** use `1.f` or `.5f` — these are bad.

## Function Calls

- Either all on one line, or wrap one argument per line (multi-line continuation indented 4 spaces).
- If the call is very short, putting many args on one line is fine.

```cpp
bool result = DoSomething(argument1, argument2, argument3);

bool result = DoSomething(averylongargument1,
                          averylongargument2,
                          averylongargument3);
```

## Braced Initializer List Format

Follow function-call formatting. Wrap if the list is long or affects readability.

```cpp
return {foo, bar, baz};                  // OK
std::vector<int> v{1, 2, 3, 4, 5, 6};    // OK on one line
std::vector<int> v = {
    1, 2, 3,                              // OK if long
};
```

## Looping and Branching Statements

- Always use braces for `if`, `else`, `for`, `while`, `do`, `switch` — including single-statement bodies.
- Exception: `if` whose body is a single `return`, `break`, `continue`, or `goto` on the next line MAY omit braces; multi-statement or `if/else` chains ALWAYS need braces.

```cpp
if (x == kFoo) return new Foo();          // OK — single return on next line
if (x == kFoo) {
  DoSomething();
}                                         // Good
if (x == kFoo) DoSomething();              // Bad — even a single statement gets braces

for (int i = 0; i < n; i++) {
  DoSomething();
}
```

Spaces: `if (condition)`, `while (condition)`, `switch (value)`. There is a space **before** the open paren but **not** after `if` itself.

## Pointer and Reference Expressions and Types

- No spaces around `.` or `->`.
- No space between variable name and `&` / `*` operators.

```cpp
x = *p;
p = &x;
x = r.y;
x = r->y;

char* c;            // Good — * with type
const std::string& str;  // Good — & with type
char *c;            // Bad
char * c;           // Bad
char* c1, *c2;      // OK syntactically but DISCOURAGED in declarations; declare separately instead.
```

`&` and `*` are written next to the **type**, not the variable name.

## Boolean Expressions

- Break long boolean expressions consistently. Operator at the **end of the line** is Google style.

```cpp
if (this_one_thing &&
    another_thing &&
    yet_another_thing) {
  ...
}
```

Use parentheses when mixing `&&` and `||` to make intent clear, even when unambiguous to the compiler.

## Return Values

- Do not use unnecessary parens: `return result;` — not `return (result);`.
- `return x;` only for simple values. For complex returns, compute first, then `return`.

## Variable and Array Initialization

`=`, `()`, and `{}` are all acceptable. Pick one per type consistently. `{}` initialization rejects narrowing conversions and works with most types — prefer `{}` for new code. Use `=` initialization to avoid type-narrowing surprises where narrowing is desired.

```cpp
int x = 3;
int x(3);
int x{3};
std::string name = "Hello";   // OK
std::string name{"Hello"};    // also good
```

## Preprocessor Directives

- The `#` is at the **start of the line** (no leading indent).

## Class Format

```
class MyClass : public BaseClass {
 public:                          // Note 1 space indent on access keywords
  MyClass();                      // ctors first, often grouped
  explicit MyClass(int var);
  ~MyClass();
  ...

 protected:                      // blank line before each access section
  void SomeProtectedFunction();

 private:
  int private_var_;               // trailing underscore
  static const int kPrivateConst;
};
```

Declaration order inside each access block:
1. `public:` types and typedefs
2. **Static constants** (kFoo)
3. **Static methods**
4. **Constructors / assignment / destructor**
5. **Everything else** (regular methods, then data members, last)

Within each group, declarations follow a logical order, not strictly alphabetical. Lifecycle-of-object ordering is generally good: ctors first, then lifecycle helpers, then operations, then accessors last.

## Constructor Initializer Lists

- All on one line if they fit.
- Otherwise, wrap: 4-space indent from class name. Comma-and-space after each.
- `: a_(a), b_(b), c_(c)` with `{}` or `{}` initializer for every field — never mix-and-match.

```cpp
// Good — fits one line.
MyClass::MyClass(int var) : some_var_(var) {
  DoSomething();
}

// Good — wrapped.
MyClass::MyClass(int a,
                 const std::string& b,
                 const std::string& c)
    : a_(a), b_(b), c_(c), some_other_var_(b + c) {
  DoSomething();
}
```

## Namespace Formatting

- A namespace declaration wraps the entire contents of a source file.
- Inside the namespace, **no extra indent** for namespace members.

```cpp
namespace foo {

// All declarations are inside the namespace, at base indent.
// No extra indent.

}  // namespace foo
```

- Use **single-line nested namespace declarations** (C++17+) rather than nesting:

```cpp
namespace foo::bar {
  // ...
}
```

## Horizontal Whitespace

- General: **no trailing whitespace**, no extra whitespace between parens and content.

| operator | spacing |
|---|---|
| assignment (`=`, `+=`, `-=`) | spaces around |
| comparison (`==`, `!=`, `<`, `>`, `<=`, `>=`) | spaces around |
| logical (`&&`, `||`) | spaces around |
| arithmetic (`+`, `-`, `*`, `/`, `%`) | spaces around; except inside `*` in casts |
| bitwise (`|`, `&`, `^`) | spaces around |
| unary (`!`, `~`, `&` (address-of), `*` (deref)) | **no space** between operator and operand |
| ternary (`?:`) | spaces around `?` and `:` |
| comma (`,`) | space after, not before |
| semicolons inside `for(;;)` | **forbidden**; use `for (;;)` (if you really need an infinite loop, prefer `while (true)`) |
| braces `{ }` | space before `{`; space after closing `}` before comma/semicolon is optional but typically absent |

## Vertical Whitespace

- Minimize blank lines. Two blank lines between functions.
- One blank line inside a function: only where it genuinely separates logical blocks.
- No blank line right after an opening `{` or before a closing `}`.
- No blank line right after access modifier (`public:`, `private:`).
- Constructor initializer lists and function bodies: a blank line is OK **inside the body**, not before `:`.

---

# Naming (Detailed Reference)

## General Philosophy

- Names convey meaning; optimizers care about behavior, not names.
- Function names often say what they **do** (verb or verb-object); classes/types say what they **are**.

| Element | Style |
|---|---|
| Type / class / concept / alias | `PascalCase` |
| Variable / function parameter | `snake_case` |
| Class data members (non-static) | `snake_case` with trailing `_` |
| Struct data members | `snake_case` (no trailing underscore) |
| Constants (`constexpr`, `const`, `enum` values) | `kPascalCase` (leading `k`) |
| Function name (regular) | `PascalCase` |
| Accessor / mutator (`GetX()`, `set_x()`) | `PascalCase` for getter; `snake_case` for setter is OK but multi-word accessors should follow function naming |
| Namespace | `snake_case` (lower, can have digits) |
| Enumerator | `kEnumName` |
| Template type parameter | `PascalCase` (it is a type) |
| Template non-type parameter | follows the category it represents (variable / constant / type name) |
| Macro (if used) | `ALL_CAPS_WITH_UNDERSCORES`, project-prefixed |

## Type Names

- Start with capital, capital letter for each new word. No underscores.
- `UrlTableErrors`, `CustomerOrder`, `MyClass`.

## Concept Names (C++20)

**Concept names follow the same rules as type names** — `PascalCase`, no Hungarian, no underscores. A concept is a type-level predicate, so it reads like one:

```cpp
template <typename T>
concept Sortable = requires(T a, T b) {
  a < b;
};

template <Sortable T>
void Sort(std::vector<T>& v) {
  std::sort(v.begin(), v.end());
}
```

`Sortable` is `PascalCase` for the same reason `MyClass` is — the convention is "type-level name = capital".

## Variable Names

- All lowercase, with underscores between words: `table_name`, `text_line`, `interesting_event`.
- Trailing underscore **only** on class data members (non-static). Struct data and global variables do **not** get the underscore.

```cpp
std::string table_name;          // local variable
size_t buffer_size_;             // class member
struct TableRecord { std::string name; int count; };  // struct member
```

## Constant Names

- Use `k` + PascalCase for variables declared as constants.
- `static constexpr int kDaysInAWeek = 7;`
- `const int kGlobalFlag = 99;` is acceptable.

## Function Names

- Regular functions: `PascalCase` — `MyExcitingFunction()`, `MyClass::MyMethod()`.
- Accessors and mutators: may additionally be named like variables — `int count(); void set_count(int);`.
- If your function is effectively a property accessor (returns a value, no side effects), call it like a variable.
- `OpenFile()` is correct (it does an action); `count()` is correct (returns a member).

## Namespace Names

- `snake_case`, no leading `k`. May contain digits.
- Avoid nested namespaces longer than necessary.
- Names declared in namespace `std` are **forbidden in user code** — do not name sub-namespaces `std` or pretend to extend the standard library.

## Enumerator Names

- Either as constants (`kEnumName`) or like macros (`ENUM_NAME`) — Google prefers `kEnumName`.

```cpp
enum class UrlTableError {
  kOk = 0,
  kNotFound,
  kNetworkError,
};
```

## Template Parameter Names

- Type parameters: `PascalCase` (they are types).
- Non-type parameters: follow the naming rules for whatever category they belong to.

## Macro Names

- Discouraged; use `inline` functions, `enums`, or `constexpr` instead.
- If you must, use `PROJECT_PREFIX_ALL_CAPS_WITH_UNDERSCORES`. Example: `MYPROJECT_ROUND()`.

## Exceptions to Naming Rules

- ISO/standard names: follow whatever convention the existing API uses (e.g. you may need to use `STL-style` macros for interop).
- Existing C/C++ code that follows a different convention: stay consistent **if** the surrounding code is genuinely following that convention and you're maintaining compatibility.
- Avoid giving names that obscure C++ idioms (e.g. don't name your template parameter `T`, even though `T` is conventional — use a descriptive PascalCase name unless brevity is clear).

## Choosing Names — Principles

- Be precise: `num_points` > `n`.
- Be consistent across the file/project; do not mix `Color` and `colour`.
- Type names should be nouns or noun phrases: `Reader`, `WidthHeight`, `PriorityQueue`.
- Function names should say what the function does: `OpenFile()`, not `DoFile()`. `IsEmpty()` not `Empty()`. Avoid `Get`/`Set` prefix when not needed.
- Avoid type-encoded names (`int_of_int`, `ClassThing`).
- Don't redefine widely-known identifiers (`errno`, `std::cout`, etc.).
- Prefer underscores between words inside an identifier; avoid Hungarian notation; avoid runs of capitals.
