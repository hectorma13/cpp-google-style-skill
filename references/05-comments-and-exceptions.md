# Comments & Exceptions to the Rules (Detailed Reference)

This is the detailed companion to the Comments section of SKILL.md and the "Exceptions to the Rules" closing chapter. Read when deciding what to comment, how to format comments, or when you genuinely need to diverge from the policy (existing code, Windows code).

---

# Comments

## Comment Style

- Either `//` for **most** comments, or `/* */` for **multi-line** comments that are intended as block-doc comments near the top of a complex file.
- Use `//` consistently within functions and for short explanations.
- Comments should be **complete, capitalized sentences** (or fragments that read naturally).
- Aim for **70–80% comment density** where the logic is non-obvious; not every line needs a comment, but complex code should be self-explanatory or commented.

```cpp
// Treat empty strings as zero. See RFC 1234.
if (s.empty()) return 0;
```

## File Comments

- **Optional** (some style guides say recommended) — a header at the top of each file briefly stating purpose and ownership.
- File header should include **copyright/owner**, not arbitrary banner comments.
- For a file that contains non-trivial logic, a one- or two-line "what does this file do" comment is helpful.

```cpp
// Copyright 2026 My Project. All rights reserved.
// Manages the on-disk cache of HTTP responses.
```

- Refrain from** ASCII art banners** ("/////// MyModule ///////"). These are discouraged.

## Class Comments

- Each non-trivial class should have a comment describing what it does, how it's used, its invariants, and any constraints.
- Class comments go **above** the class declaration:

```cpp
// A Token represents a single lexed token in the source code.
// It owns its data via std::string and is movable but not copyable.
class Token { ... };
```

- For small classes, the class name and a one-line comment is enough.

## Function Comments

- For **non-trivial** public functions, a comment describes:
  - What the function does.
  - Input expectations (especially preconditions).
  - What is returned.
  - Side effects, if any.
  - Anything tricky.

```cpp
// Returns the lower-cased version of |s|, with leading/trailing whitespace stripped.
// Does not handle Unicode; assumes ASCII.
std::string Normalize(const std::string& s);
```

- For **one-line functions**, the comment may be omitted if the function name reads naturally.
- Place comments **above** the function declaration — or above the definition if declaration is brief.
- Override the rule by attaching "intention revealing" arguments or **`absl::Nonnull(...)`** annotations; if behavior is documented elsewhere, no comment needed.

## Variable Comments

- Tricky non-trivial variables get a comment.
- For class data members with universal scope (`static constexpr int kFoo = 5;`), the comment can be inline:

```cpp
static constexpr int kMaxRetries = 3;  // Conservative upper bound to avoid retry storms.
```

## Implementation Comments

- For tricky or non-obvious code sections, add a comment that explains **why** (not what).
- Example: "// Remove the trailing newline before hashing, since we wrote it ourselves."

## Punctuation, Spelling, and Grammar

- Capitalize the first word of each comment.
- End comments in **periods** (for sentences / fragments).
- Aim for **perfectly-formed sentences** when the comment isn't a one-line aside.
- Reference identifiers **exactly** as they appear (the right case).
- It may be acceptable to omit the period on short fragment comments.

A short slash-slash comment like `// Treat empty strings as zero.` reads naturally and is preferred over inconsistent fragments.

## TODO Comments

- Use `TODO(name):` (with a name) for TODOs that may be picked up by someone specific:

```cpp
// TODO(johndoe): refactor once we have the new parser.
```

- `FIXME:` is **deprecated**; use `TODO(name):` instead.
- Associated date is encouraged: `TODO(2026-03-15): use absl::Status across the codebase.`

## Doxygen / Block Comments

- Doxygen-style block comments (`/** */`, `///`) should be used **only** if your project has explicit Doxygen generation, OR for documenting public APIs of a library.
- Otherwise, use `//` to keep it consistent.

---

# Exceptions to the Rules

## Existing Non-conformant Code

- When **modifying existing** code that does not follow this style guide, you may diverge from the guide **only** in places where the existing style is genuinely entrenched.
- Example: if existing code uses 4-space indentation, and you're modifying only a small region, you may keep 4 spaces in that region rather than reformat the entire file.
- **General policy:** for new files, follow the guide strictly. For modifications of broad reach, also follow the guide strictly. For minimal local modifications, you may match the existing style.

## Windows Code

Some restrictions are relaxed in Windows-targeted code:

- **Windows code must follow this guide.**
- A small number of Windows-specific rules differ:
  - **Hungarian notation** (e.g. `dwSize`, `bFlag`) is **forbidden** — even in Windows code.
  - Use **Google-style header guards** as the primary; you may use **`#pragma once`** as an additional guard for Windows compilers.
  - Specifically for Windows / COM / ATL / WTL code:
    - Multiple implementation inheritance is allowed only when ATL/COM policy requires it.
    - Preprocessor macros (`#define`**`STDMETHOD`**`-style`) are allowed if they bridge a nonstandard compiler-extension.
  - The use of `bstr_t` is allowed, but prefer `std::string` everywhere else.

## Severity of a rule

The Google guide itself acknowledges that some rules can be broken for genuine reasons: pre-existing code, performance, external compatibility. The rule of thumb is:

- Default: follow the rule.
- Justified exceptions: have a comment explaining why you broke it.

This style guide is **strong but pragmatic**. It encodes many years of Google internal C++ practice. Following it strictly is not "free"; it's an investment in long-term maintenance cost — code lives longer than you think.
