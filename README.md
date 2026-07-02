# google-cpp-style

A skill that enforces the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) inside Claude Code, Cursor, Windsurf, Codex, Aider, and any other SKILL-aware agent. Once loaded, the agent applies the rules to every byte of C++ it writes or reviews.

## Install

```bash
npx skills add https://github.com/hectorma13/cpp-google-style-skill --skill google-cpp-style --skill google-cpp-style
# bun
bunx skills add https://github.com/hectorma13/cpp-google-style-skill --skill google-cpp-style
# pnpm
pnpm dlx skills add https://github.com/hectorma13/cpp-google-style-skill --skill google-cpp-style
```

`<owner>` is your GitHub user or org — e.g. `me/cpp-google-style-skill` once you've forked.

The installer is the [`skills` CLI](https://github.com/vercel-labs/skills) from Vercel Labs. `npx` will fetch it on demand; nothing to install globally first. Pass `-y` to skip prompts.

## What it does

The skill is a packaged set of markdown. The agent reads [`SKILL.md`](SKILL.md) first (one page, the cheatsheet and the eight Google principles), then dives into [`references/`](references/) only when it has to. Worked examples live in [`examples/url_table.h`](examples/url_table.h) and `examples/url_table.cc` — those two files are written against every rule this skill encodes and are the fastest way to understand the bar.

After loading, the agent will, on its own:

- refuse to type C++ exceptions, RTTI, C-style casts, unscoped `enum`, or C++20 modules unless the user explicitly opts out and the agent adds a `// Rationale:` comment locally;
- apply Google naming (`kPascalCase` constants, `PascalCase` types/concepts, `snake_case` variables, trailing `_` on class members);
- alphabetize `#include` groups in the canonical order (related header → C system → C++ std → other libs → project);
- default to `absl::StatusOr<T>` for fallible APIs;
- close namespaces with `}  // namespace foo`, indent with 2 spaces, no tabs, 80-char lines.

## What it won't do

- Won't write `using namespace std`.
- Won't pick Boost where Abseil is canonical (and vice versa where your project is the other way).
- Won't migrate your codebase. It writes new code and reviews code you point it at.
- Won't enforce your internal naming if it diverges from Google's — say so explicitly and it'll break the rule with a `// Rationale:` comment.
- Won't try to compile C++23 features. The target is C++20.

## Layout

```
SKILL.md                                  entrypoint (one page)
references/01-formatting-and-naming.md    line length, brace style, naming tables
references/02-headers-and-scoping.md      IWYU, #define guards, namespaces
references/03-classes-and-functions.md    class shape, copy/move, functions
references/04-restrictions-and-modern-cpp.md   RTTI ban, casting, lambdas, modules, cpplint
references/05-comments-and-exceptions.md  comment style, TODO policy, Windows code
examples/url_table.h                      canonical header, every rule applied
examples/url_table.cc                     canonical implementation
examples/CMakeLists.txt                   C++20 + abseil build
LICENSE                                   Apache 2.0
```

## Useful flags

```bash
# install to ~/.<agent>/skills/ instead of project-local
npx skills add https://github.com/hectorma13/cpp-google-style-skill --skill google-cpp-style -g
# skip prompts (CI)
npx skills add https://github.com/hectorma13/cpp-google-style-skill --skill google-cpp-style -y
# install into one specific agent
npx skills add https://github.com/hectorma13/cpp-google-style-skill --skill google-cpp-style --agent cursor
# peek without installing
npx skills add https://github.com/hectorma13/cpp-google-style-skill --list
# list what's installed
npx skills list
# refresh
npx skills update google-cpp-style
# uninstall
npx skills remove google-cpp-style
```

## Compatibility

The skill lands wherever the chosen agent reads skill files from. These are the paths the installer writes to by default:

- Claude Code — `.claude/skills/` (project) or `~/.claude/skills/` (global)
- Cursor — `~/.cursor/skills/`
- Windsurf — `~/.codeium/windsurf/skills/`
- Codex CLI — `~/.codex/skills/`

If you don't see your agent here, point `--agent <name>` at it, or copy `SKILL.md` + `references/` + `examples/` into whatever folder your agent watches.

## If it doesn't work

`npx skills` is fetched on demand; if it fails, check `node --version` (need ≥ 18) and that you can reach npm. Per-flag help is `npx skills add --help`. To see what the installer actually does in a repo before committing to it: `npx skills add <owner>/cpp-google-style-skill --list`.

## License

Apache 2.0 — see [`LICENSE`](LICENSE). Style content © Google; the canonical upstream is [google.github.io/styleguide/cppguide.html](https://google.github.io/styleguide/cppguide.html) and this skill is an external packaging of it.

## Contributing

PRs welcome. Add a rule by editing `SKILL.md` + the matching `references/NN-*.md` with one Good and one Bad example. Add a worked example by dropping a `.h` + matching `.cc` under `examples/<concept>/`, copying the header guard format from `examples/url_table.h`. Where this skill and the canonical disagree, the canonical wins — please open a PR with the fix.
