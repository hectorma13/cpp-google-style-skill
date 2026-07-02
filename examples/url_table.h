// Copyright 2026 The Project Authors. All rights reserved.
//
// Example header for a "URL table" — demonstrates nearly every Google C++ Style
// rule applied together. When in doubt about how a rule looks in practice, read
// this file end to end.

#ifndef GOOGLE_CPP_STYLE_EXAMPLES_URL_TABLE_H_
#define GOOGLE_CPP_STYLE_EXAMPLES_URL_TABLE_H_

#include <memory>
#include <string>
#include <unordered_map>

// `absl::StatusOr<T>` is the canonical Google-style way to return "a value of
// type T, or an error". This skill's dealbreakers ban C++ exceptions, so
// fallible functions return a StatusOr instead of throwing. To compile this
// file you need Abseil installed (https://abseil.io) and discoverable via
// your build system. In C++23 projects that don't use Abseil, the closest
// stdlib equivalent is std::expected; until then, StatusOr is the standard
// pick on the Google ecosystem.
#include "absl/status/statusor.h"

namespace google_cpp_style::examples {

// Errors that `UrlTable` operations can return. Note the leading `k` on every
// enumerator and the scoped `enum class`.
enum class UrlTableError {
  kOk = 0,
  kNotFound,
  kAlreadyExists,
  kInvalidUrl,
};

// UrlTable is a process-wide cache of canonicalized URLs and the metadata
// associated with them. Owned by the application as a singleton via
// `GetUrlTable()`; not thread-safe for writes.
//
// Ownership: this class owns its `metadata_` map and exposes read-only views.
// Adding a URL copies the metadata into the table.
class UrlTable {
 public:
  // Public types and aliases go first.
  using MetadataMap = std::unordered_map<std::string, std::string>;

  // Static constants next.
  static constexpr int kMaxUrlLength = 1024;

  // Constructors / assignment / destructor.
  UrlTable();
  ~UrlTable();
  UrlTable(const UrlTable&) = delete;
  UrlTable& operator=(const UrlTable&) = delete;
  UrlTable(UrlTable&&) = default;
  UrlTable& operator=(UrlTable&&) = default;

  // Operations.
  absl::StatusOr<bool> Add(const std::string& url,
                           const std::string& metadata);

  // Accessors read like variables. We return a raw pointer (`nullptr` for
  // "not present") instead of a reference — exceptions are banned by the
  // style guide, and `.at()` would throw on a miss. Borrow = raw pointer.
  const std::string* metadata_for(const std::string& url) const {
    const auto it = metadata_.find(url);
    if (it == metadata_.end()) return nullptr;
    return &it->second;
  }

  int size() const { return metadata_.size(); }

 private:
  std::string Normalize(const std::string& url) const;
  bool OwnsMetadata(const std::string& url) const;

  MetadataMap metadata_;
};

// Free function in the same namespace — preferred over a public static method
// when no private state is needed.
absl::StatusOr<std::unique_ptr<UrlTable>> GetUrlTable();

}  // namespace google_cpp_style::examples

#endif  // GOOGLE_CPP_STYLE_EXAMPLES_URL_TABLE_H_
