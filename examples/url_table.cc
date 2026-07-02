// Copyright 2026 The Project Authors. All rights reserved.
//
// Implementation of the UrlTable example. The companion header lives in
// url_table.h. Read SKILL.md and the references/ files for the rules this
// file applies; in particular:
//
//   - include order is strict (related, C, C++, other libs, project)
//   - IWYU: every header used is included directly
//   - the entire file is wrapped in a namespace
//   - errors are returned via absl::Status / absl::StatusOr — no exceptions
//   - locals are declared at the narrowest scope that uses them

#include "examples/url_table.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <utility>

// The two Abseil headers below are Google's small-string idioms:
//   - absl::StrCat      : concatenate strings without std::ostringstream overhead
//                          (also generic over numeric types).
//   - absl::StripSuffix : "foo/" -> "foo" without writing a substring loop.
// Both are available because this skill's canonical ecosystem is Abseil. If
// you don't want to depend on Abseil, plain std::string concatenation and a
// manual `s.rfind('/')` substring check work too, at a small readability cost.
#include "absl/strings/str_cat.h"
#include "absl/strings/strip.h"

namespace google_cpp_style::examples {

UrlTable::UrlTable() = default;
UrlTable::~UrlTable() = default;

// Normalize strips any trailing slash and lowercases the URL. Hot-path code;
// kept short on purpose.
std::string UrlTable::Normalize(const std::string& url) const {
  std::string out = absl::StripSuffix(url, "/");
  std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return out;
}

bool UrlTable::OwnsMetadata(const std::string& url) const {
  // Declared at the narrow scope that uses it.
  const auto it = metadata_.find(url);
  return it != metadata_.end();
}

absl::StatusOr<bool> UrlTable::Add(const std::string& url,
                                  const std::string& metadata) {
  if (url.size() > static_cast<size_t>(kMaxUrlLength)) {
    return absl::InvalidArgumentError("URL exceeds kMaxUrlLength");
  }
  const std::string canonical_url = Normalize(url);

  // `try_emplace` returns the existing iterator when the key was already
  // present; `inserted` is `false` in that case.
  const auto [it, inserted] = metadata_.try_emplace(canonical_url, metadata);
  if (!inserted && it->second != metadata) {
    return absl::AlreadyExistsError(absl::StrCat(
        "URL already registered with different metadata: ", canonical_url));
  }
  return inserted;
}

absl::StatusOr<std::unique_ptr<UrlTable>> GetUrlTable() {
  // Construct via a local rather than `make_unique` so the factory could
  // fail (e.g. absl::StatusOr) without throwing.
  auto table = std::make_unique<UrlTable>();
  return table;
}

}  // namespace google_cpp_style::examples
