option(toml11_INSTALL "" OFF)

FetchContent_Declare(
  toml11
  GIT_REPOSITORY  https://github.com/ToruNiina/toml11.git
  GIT_TAG         v3.8.0
  GIT_SHALLOW     ON
)
FetchContent_MakeAvailable(toml11)

mark_as_advanced(
  CMAKE_CXX_EXTENSIONS
  CMAKE_CXX_STANDARD_REQUIRED
  toml11_BUILD_TEST
  toml11_INSTALL
  toml11_TEST_WITH_ASAN
  toml11_TEST_WITH_UBSAN
  TOML11_USE_UNRELEASED_TOML_FEATURES
)
