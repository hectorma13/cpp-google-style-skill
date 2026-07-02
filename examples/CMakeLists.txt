# Minimal CMakeLists.txt for the url_table example.
#
# Shows the build-side conventions that pair with the Google C++ Style rules.
# In production: extend with absl/strings, absl/status, project-wide flags,
# cpplint pre-submit, and a separate test target.
#
# DEPENDENCY — Abseil. This example targets the Google C++ ecosystem, so
# url_table.{h,cc} use absl::StatusOr, absl::StrCat and absl::StripSuffix.
# Abseil is NOT in the C++ standard library; you must install it from
# https://abseil.io (apt, vcpkg, conan, or a manual git clone). Without
# Abseil, the example won't compile.
#
# The header guard in url_table.h uses the prefix GOOGLE_CPP_STYLE. If you
# copy this template into a different project, rename the file's guard and
# the `namespace google_cpp_style::examples` blocks accordingly.

cmake_minimum_required(VERSION 3.16)
project(google_cpp_style_examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(url_table
  url_table.cc
)

# The header is included as "examples/url_table.h" from any consumer, so the
# include root is the parent directory (here: the project root). Adjust to
# match your layout.
target_include_directories(url_table
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Warning flags common to Google C++ projects. Project may extend.
target_compile_options(url_table
  PRIVATE
    -Wall -Wextra -Wpedantic
)

# abseil is the typical third-party dependency for projects using
# absl::StatusOr, absl::StrCat, etc. Adjust find_package call to match your
# build system (vcpkg, conan, system package, etc.).
find_package(absl CONFIG REQUIRED)
target_link_libraries(url_table
  PUBLIC
    absl::status
    absl::statusor
    absl::strings
)
